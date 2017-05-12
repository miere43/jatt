#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "application_state.h"

#include "add_activity_dialog.h"
#include "edit_activity_field_dialog.h"

#include <QtAlgorithms>
#include <QDebug>
#include <QMessageBox>
#include <QBoxLayout>
#include <QFileDialog>
#include <QSettings>

void hotkeyCallback(Hotkey* hotkey, void* userdata);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_activityListModel(new ActivityListModel)
    , m_activityVisualizer(new ActivityVisualizer)
    , m_activityItemDelegate(new ActivityItemDelegate)
{
    g_app.setMainWindow(this);
    ui->setupUi(this);

#ifndef APP_DEBUG_DB
    this->setWindowTitle(g_app.appTitle);
#endif

    ui->activitiesListView->setModel(m_activityListModel);
    ui->activitiesListView->setItemDelegate(m_activityItemDelegate);
    ui->activitiesListView->setContextMenuPolicy(Qt::CustomContextMenu);

    setViewDay(getCurrentDayIndex());

    ui->verticalLayout_4->addWidget(m_activityVisualizer);

    connect(&m_activityRecorder, &ActivityRecorder::recordEvent,
            this, &MainWindow::activityRecorderRecordEvent);
    connect(ui->activitiesListView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::selectedActivityChanged);
    connect(ui->activitiesListView, &QListView::customContextMenuRequested,
            this, &MainWindow::activitiesListViewMenuRequested);
    connect(ui->activitiesListView, &QListView::doubleClicked,
            this, &MainWindow::activitiesListViewDoubleClicked);

    m_changePageLeftShortcut = new QShortcut(QKeySequence(Qt::Key_Left), this);
    m_changePageRightShortcut = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(m_changePageLeftShortcut, &QShortcut::activated,
            this, &MainWindow::changePageLeftShortcutActivated);
    connect(m_changePageRightShortcut, &QShortcut::activated,
            this, &MainWindow::changePageRightShortcutActivated);

    m_activityVisualizer->setTimelineRenderMode(ActivityVisualizer::Full);

    m_activityMenu.addAction(ui->addActivityAction);

    connect(ui->deleteActivityAction, &QAction::triggered,
            this, &MainWindow::deleteSelectedActivityTriggered);

    g_app.m_pluginManager.initialize();

    g_app.database()->loadActivityInfos();
    QList<ActivityInfo*> list = g_app.database()->activityInfos();
    m_listMenus.reserve(list.count());
    for (ActivityInfo* info : list) {
        m_listMenus.insert(info, createActivityInfoMenu(info));
    }

    addQuickActivityButtons();
    readAndApplySettings();

    // @TODO: load key combination from settings
    m_recorderHotkey = new Hotkey((HWND)this->winId(), 1, Qt::ControlModifier, Qt::Key_Space, hotkeyCallback, (void*)this);
    if (!m_recorderHotkey->isActive()) {
#ifdef QT_DEBUG
        qDebug() << "Unable to register hotkey: " + m_recorderHotkey->errorMessage();
#else
        QMessageBox::critical(this, "Error", "Unable to register hotkey: " + m_recorderHotkey->errorMessage());
#endif
    }
}

void hotkeyCallback(Hotkey* hotkey, void* userdata) {
    MainWindow* window = (MainWindow*)userdata;
    window->handleHotkey(hotkey);
}

void MainWindow::handleHotkey(Hotkey *hotkey) {
    if (hotkey == m_recorderHotkey) {
        if (m_activityRecorder.isRecording()) {
            m_activityRecorder.stop();
        } else {
            if (m_lastActiveActivity != nullptr) {
                m_activityRecorder.record(m_lastActiveActivity);
            }
        }
    }
}

inline qint64 clamp(qint64 value, qint64 min, qint64 max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

qint64 visibleActivitiesDuration(const QVector<Activity*>& activities, qint64 lowBound, qint64 highBound) {
    Q_ASSERT(lowBound < highBound);

    qint64 sum = 0;
    for (const Activity* activity : activities) {
        for (const Interval& interval : activity->intervals) {
            qint64 ist = clamp(interval.startTime, lowBound, highBound);
            qint64 iet = clamp(interval.endTime,   lowBound, highBound);
            if (ist > iet) {
                qWarning() << "invalid time";
                continue;
            }
            sum += iet - ist;
        }
    }

    return sum;
}

void MainWindow::changePageLeftShortcutActivated() {
    if (m_viewDay > 0)
        setViewDay(m_viewDay - 1);
}

void MainWindow::changePageRightShortcutActivated() {
    setViewDay(m_viewDay + 1);
}

QMenu* MainWindow::createActivityInfoMenu(ActivityInfo *info) {
    QMenu* m = new QMenu(this);
    m->addAction(ui->editActivityAction);
    m->addAction(ui->deleteActivityAction);
    m->addSeparator();
    m->addAction(ui->joinNextActivityAction);
    m->addAction(ui->splitActivityAction);
    m->addSeparator();

    int i = 0;
    for (const QString& fieldName : info->fieldNames) {
        QAction* action = m->addAction(QString("Edit \"") + fieldName + "\"");
        // @TODO: this is so eh, maybe there is a better solution?
        action->setProperty("fieldIndex", QVariant::fromValue<int>(i));
        connect(action, &QAction::triggered,
                this, &MainWindow::activityMenuItemActionTriggered);
        ++i;
    }
    return m;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_activityListModel;
    delete m_recorderHotkey;
}

void MainWindow::editActivityFieldDialogFinished(int result) {
    QObject* sender = QObject::sender();
    EditActivityFieldDialog* dialog = qobject_cast<EditActivityFieldDialog*>(sender);
    Q_ASSERT(dialog);

    if (result == QDialog::Accepted) {
        Activity* activity = dialog->activity();
        int i = activity->info->fieldNames.indexOf(dialog->fieldIndex());
        Q_ASSERT(i != -1);
        Q_ASSERT(activity->info->fieldNames.count() == activity->fieldValues.count());
        activity->fieldValues[i] = dialog->newValue().toString();
        m_activityListModel->dataChangedHint(activity);

        g_app.database()->saveActivity(activity);
    }

    dialog->deleteLater();
}

void MainWindow::showEditActivityFieldDialog(Activity* activity, int fieldIndex) {
    EditActivityFieldDialog* dialog = new EditActivityFieldDialog(activity, fieldIndex, this);
    connect(dialog, &EditActivityFieldDialog::finished,
            this, &MainWindow::editActivityFieldDialogFinished);
    dialog->show();
}

void MainWindow::activityMenuItemActionTriggered(bool checked) {
    Q_UNUSED(checked);

    QObject* sender = QObject::sender();
    QAction* action = qobject_cast<QAction*>(sender);
    Q_ASSERT(action);

    Activity* activity = selectedActivity();
    Q_ASSERT(activity);
    int fieldIndex = action->property("fieldIndex").value<int>();

    showEditActivityFieldDialog(activity, fieldIndex);
}

void MainWindow::activitiesListViewMenuRequested(const QPoint &pos)
{
    QModelIndex item = ui->activitiesListView->indexAt(pos);
    if (!item.isValid()){
        m_activityMenu.exec(ui->activitiesListView->mapToGlobal(pos));
    } else {
        Activity* activity = (Activity*)item.data(Qt::UserRole).value<void*>();
        Q_ASSERT(activity);
        Q_ASSERT(activity->info);

        // @TODO: can move QMenu* from hashtable to ActivityInfo struct (because all AI's have a menu)
        QMenu* menu = m_listMenus.value(activity->info, nullptr);
        if (menu == nullptr) {
            Q_ASSERT(false); // All Activity Info instances should have a menu associated with them.
        }

        menu->exec(ui->activitiesListView->mapToGlobal(pos));
    }
}

void MainWindow::activitiesListViewDoubleClicked(const QModelIndex &index) {
    if (index.isValid()) {
        Activity* activity = (Activity*)index.data(Qt::UserRole).value<void*>();
        Q_ASSERT(activity);

        if (activity->info->fieldNames.count() > 0) {
            showEditActivityFieldDialog(activity, 0);
        }
    }
}

void MainWindow::on_addActivityAction_triggered()
{
    showAddActivityDialog();
}

void MainWindow::showAddActivityDialog()
{
    AddActivityDialog* dialog = new AddActivityDialog(this);
    connect(dialog, &AddActivityDialog::finished,
            this, &MainWindow::addActivityDialogFinished);
    dialog->exec();
}

void MainWindow::addActivityDialogFinished(int result)
{
    QObject* sender = QObject::sender();
    AddActivityDialog* dialog = qobject_cast<AddActivityDialog*>(sender);
    Q_ASSERT(dialog);

    dialog->deleteLater();

    if (result == QDialog::Accepted)
    {
        Activity* activity = dialog->constructActivity();
        if (activity == nullptr)
        {
            QMessageBox::critical(this, "Error", "Unable to create activity.");
            return;
        }

        // @TODO: now we assume that setViewTimePeriod was called already, that may change in the future.
        if (activity->belongsToTimePeriod(m_currentViewTimePeriodStartTime, m_currentViewTimePeriodEndTime))
        {
            m_activityListModel->addActivity(activity);

            m_currentViewTimePeriodActivities.append(activity);
            m_activityVisualizer->update();
        }
    }
}

#include <QtAlgorithms>
bool activityLessThan(const Activity* a, const Activity* b) {
    return a->startTime < b->startTime;
}

void activitySort(QVector<Activity*>* vec) {
    qSort(vec->begin(), vec->end(), activityLessThan);
}

void MainWindow::setViewTimePeriod(qint64 startTime, qint64 endTime)
{
    m_currentViewTimePeriodStartTime = startTime;
    m_currentViewTimePeriodEndTime   = endTime;

    m_currentViewTimePeriodActivities.clear();

    g_app.database()->loadActivitiesBetweenStartAndEndTime(&m_currentViewTimePeriodActivities, startTime, endTime, true);

    if (m_activityRecorder.isRecording())
    {
        Activity* currentActivity = m_activityRecorder.activity();
        Q_ASSERT(currentActivity);

        if (currentActivity->belongsToTimePeriod(startTime, endTime)) {
            if (!m_currentViewTimePeriodActivities.contains(currentActivity)) {
                // qDebug() << "added current activity " << currentActivity->fieldValues[0];
                m_currentViewTimePeriodActivities.append(currentActivity);
            }
        }
    }

    activitySort(&m_currentViewTimePeriodActivities);

    m_activityListModel->clear();
    m_activityListModel->setTimePeriod(startTime, endTime);
    m_activityListModel->addActivities(m_currentViewTimePeriodActivities);

    m_activityVisualizer->setTimePeriod(startTime, endTime, &m_currentViewTimePeriodActivities);
    updateVisibleActivitiesDurationLabel();
}

void MainWindow::updateVisibleActivitiesDurationLabel() {
    ui->timePeriodTotalTimeLabel->setText(
        createDurationStringFromMsecs(
            visibleActivitiesDuration(m_currentViewTimePeriodActivities,
                                      m_currentViewTimePeriodStartTime,
                                      m_currentViewTimePeriodEndTime)));
}

void MainWindow::updateActivityDurationLabel() {
    // @TODO: this is weird stuff
    Activity* currentActivity;
    if (m_activityRecorder.isRecording()) {
        currentActivity = m_activityRecorder.activity();
    } else {
        currentActivity = selectedActivity();
    }

    if (currentActivity == nullptr) {
        ui->activityDurationLabel->setText(QStringLiteral("00:00:00"));
    } else {
        ui->activityDurationLabel->setText(createDurationStringFromMsecs(currentActivity->duration()));
    }
}

void MainWindow::setViewDay(qint64 day)
{
    if (day < 0) day = 0;

    qint64 startTime = (day * 86400000LL) - g_app.localOffsetFromUtc();
    qint64 endTime = startTime + 86400000LL;
    setViewTimePeriod(startTime, endTime);

    ui->dayLabel->setText(QDateTime::fromMSecsSinceEpoch(startTime, Qt::LocalTime).toString("MMM <b>dd</b>, yyyy"));
    if (day == 0)
    {
        ui->prevDayButton->setEnabled(false);
    }
    else
    {
        ui->prevDayButton->setEnabled(true);
    }

    m_viewDay = day;
}

void MainWindow::on_nextDayButton_clicked()
{
    setViewDay(m_viewDay + 1);
}

void MainWindow::on_prevDayButton_clicked()
{
    if (m_viewDay > 0)
        setViewDay(m_viewDay - 1);
}

void MainWindow::on_startActivityButton_clicked()
{
    if (m_activityRecorder.isRecording())
    {
        m_activityRecorder.stop();
    }
    else
    {
        Activity* activity = selectedActivity();
        if (!activity)
        {
            QMessageBox::critical(this, "Error", "Select activity to start.");
            return;
        }

        // qDebug() << "record:" << activity->id;
        m_activityRecorder.record(activity);
        // !! Don't do anything here, activityRecorderRecordEvent signal will be fired right now.
    }
}

Activity* MainWindow::selectedActivity() const
{
    QModelIndex index = selectedActivityIndex();
    return index.isValid() ? (Activity*)index.data(Qt::UserRole).value<void*>() : nullptr;
}

QModelIndex MainWindow::selectedActivityIndex() const
{
    QModelIndexList selection = ui->activitiesListView->selectionModel()->selection().indexes();
    if (selection.count() == 0)
    {
        return QModelIndex();
    }
    return selection.at(0);
}

void MainWindow::activityRecorderRecordEvent(ActivityRecorderEvent event)
{
    // qDebug() << m_activityRecorder.activity()->endTime;
    Activity* currentActivity = m_activityRecorder.activity();
    // Interval* currentInterval = m_activityRecorder.interval();

    if (event == ActivityRecorderEvent::RecordingStarted)
    {
        ui->startActivityButton->setText("Stop");
        // @TODO: now we assume that setViewTimePeriod was called already, that may change in the future.
        if (currentActivity->belongsToTimePeriod(m_currentViewTimePeriodStartTime, m_currentViewTimePeriodEndTime))
        {
            if (!m_currentViewTimePeriodActivities.contains(currentActivity)) {
                m_currentViewTimePeriodActivities.append(currentActivity);
            }
            m_activityVisualizer->selectActivity(currentActivity);
            m_activityItemDelegate->setCurrentActivity(currentActivity);
        }

        m_lastActiveActivity = currentActivity;
        this->setWindowTitle("(" + currentActivity->info->name + ") " + g_app.appTitle);
    }
    else if (event == ActivityRecorderEvent::RecordingStopped)
    {
        g_app.database()->saveActivity(currentActivity);

        ui->startActivityButton->setText("Continue"); // @HARDCODE
        m_activityItemDelegate->setCurrentActivity(nullptr);

        this->setWindowTitle(g_app.appTitle);
    }

    if (event == ActivityRecorderEvent::RecordingStarted || event == ActivityRecorderEvent::UpdateUITimer)
    {
        Q_ASSERT(currentActivity);
        updateActivityDurationLabel();
    }

    if (currentActivity->belongsToTimePeriod(m_currentViewTimePeriodStartTime, m_currentViewTimePeriodEndTime)) {
        updateVisibleActivitiesDurationLabel();
    }

    m_activityVisualizer->update();
}

void MainWindow::selectedActivityChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);

    Activity* activity = nullptr;
    QModelIndexList indexes = selected.indexes();
    if (indexes.count() > 0)
    {
        const QModelIndex& index = indexes.at(0);
        if (index.isValid())
            activity = static_cast<Activity*>(index.data(Qt::UserRole).value<void*>());
    }

    // @TODO: should move to from here to open menu function
    {
        bool enableSplitActivityAction;
        if (activity == nullptr) {
            enableSplitActivityAction = false;
        } else {
            enableSplitActivityAction = activity->intervals.count() > 1;
        }
        ui->splitActivityAction->setEnabled(enableSplitActivityAction);
    }

    if (m_activityRecorder.isRecording()) return;

    if (activity == nullptr)
    {
        ui->activityDurationLabel->setText(QStringLiteral("00:00:00"));
        m_activityVisualizer->selectActivity(nullptr);
    }
    else
    {
        ui->activityDurationLabel->setText(createDurationStringFromMsecs(activity->duration()));
        m_activityVisualizer->selectActivity(activity);
    }
}

void MainWindow::deleteSelectedActivityTriggered(bool checked)
{
    Q_UNUSED(checked);

    QModelIndexList selection = ui->activitiesListView->selectionModel()->selection().indexes();
    if (selection.count() == 0) {
        QMessageBox::critical(this, "Error", "Activity is not selected.");
        return;
    }

    Activity* activity = ((Activity*)selection.at(0).data(Qt::UserRole).value<void*>());
    Q_ASSERT(activity);

    if (m_activityRecorder.isRecording() && m_activityRecorder.activity() == activity) {
        QMessageBox::critical(this, "Error", "Cannot delete currently recording activity");
        return;
    }

    removeActivityFromView(activity);

    if (activity->id >= 0) {
        g_app.database()->deleteActivity(activity->id);
    }

    g_app.m_activityAllocator.deallocate(activity);
}

void MainWindow::removeActivityFromView(Activity *activity) {
    if (m_activityRecorder.isRecording()) {
        Q_ASSERT(m_activityRecorder.activity() != activity);
    }

    if (m_lastActiveActivity == activity) {
        m_lastActiveActivity = nullptr;
    }

    int index = m_currentViewTimePeriodActivities.indexOf(activity);
    if (index != -1) {
        m_currentViewTimePeriodActivities.remove(index);
        m_activityListModel->removeActivity(activity);
        m_activityVisualizer->update();
        updateVisibleActivitiesDurationLabel();
    }
}

const QVector<Activity*>& MainWindow::currentActivities() const {
    return m_currentViewTimePeriodActivities;
}

void MainWindow::startQuickActivityButtonClicked()
{
    QObject* sender = QObject::sender();
    QPushButton* button = qobject_cast<QPushButton*>(sender);
    Q_ASSERT(button);

    ActivityInfo* info = (ActivityInfo*)button->property("activityInfo").value<void*>();
    Q_ASSERT(info);

    startQuickActivity(info);
}

void MainWindow::addQuickActivityButtons()
{/*
    g_app.database()->loadActivityInfos();*/
    QList<ActivityInfo*> infos = g_app.database()->activityInfos();
    for (ActivityInfo* info : infos) {
        QPushButton* button = new QPushButton(this);
        button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        button->setCursor(QCursor(Qt::PointingHandCursor));
        button->setStyleSheet(
                    QStringLiteral(
                    "QPushButton {"
                    "    border: none;"
                    "    background: transparent;"
                    "}"
                    ":hover {"
                    "    color: #2800FF;"
                    "    text-decoration: underline;"
                    "}"
                    )
                    );
        button->setText(info->name);
        connect(button, &QPushButton::clicked,
                this, &MainWindow::startQuickActivityButtonClicked);
        button->setProperty("activityInfo", QVariant::fromValue<void*>(info));
        ui->quickActivityLayout->addWidget(button);
    }
}

void MainWindow::startQuickActivity(ActivityInfo* info) {
    Q_ASSERT(info);
    if (m_activityRecorder.isRecording()) {
        QMessageBox::critical(this, "Error", "Unable to create quick activity because already recording.");
        return;
    }

    Activity* activity = g_app.m_activityAllocator.allocate();
    activity->id = -1;
    activity->info = info;

    if (info->fieldNames.count() > 0) {
        activity->fieldValues.reserve(info->fieldNames.count());
        for (int i = 0; i < info->fieldNames.count(); ++i) {
            if (i == 0) {
                activity->fieldValues.append(info->name);
            } else {
                activity->fieldValues.append(QStringLiteral(""));
            }
        }
    }

    activity->startTime = activity->endTime = getCurrentDateTimeUtc();

    g_app.database()->saveActivity(activity);

    if (activity->belongsToTimePeriod(m_currentViewTimePeriodStartTime, m_currentViewTimePeriodEndTime)) {
        QModelIndex index = m_activityListModel->addActivity(activity);
        ui->activitiesListView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
    } else {
        setViewDay(getCurrentDayIndex());
    }

    m_activityRecorder.record(activity);
}

// @TODO: make transaction-based
void MainWindow::on_joinNextActivityAction_triggered()
{
    QModelIndexList selection = ui->activitiesListView->selectionModel()->selection().indexes();
    if (selection.count() == 0)
    {
        QMessageBox::critical(this, "Error", "Activity is not selected.");
        return;
    }

    Activity* sel = ((Activity*)selection.at(0).data(Qt::UserRole).value<void*>());
    Q_ASSERT(sel);

    int row = selection.at(0).row();
    QModelIndex idx = m_activityListModel->index(row + 1);
    if (!idx.isValid()) {
        QMessageBox::critical(this, "Error", "Next item is invalid.");
        return;
    }

    Activity* next = (Activity*)m_activityListModel->data(idx, Qt::UserRole).value<void*>();
    if (!next) {
        QMessageBox::critical(this, "Error", "Next item is invalid (2).");
        return;
    }

    if (next->id <= 0) {
        QMessageBox::critical(this, "Error", "Activity is not uploaded yet.");
        return;
    }

    if (sel->info != next->info) {
        QMessageBox::critical(this, "Error", "Activity Infos should match.");
        return;
    }

    if (!canModifyActivityIntervals(next) || !canModifyActivityIntervals(sel)) {
        QMessageBox::critical(this, "Error", "Cannot modify intervals right now.");
        return;
    }

    for (const Interval& interval : next->intervals) {
        sel->intervals.append(interval);
    }

    removeActivityFromView(next);
    updateActivityDurationLabel();

    g_app.database()->saveActivity(sel);
    g_app.database()->deleteActivity(next->id);
    g_app.m_activityAllocator.deallocate(next);
}

bool MainWindow::canModifyActivityIntervals(Activity *activity) const
{
    Q_ASSERT(activity);
    if (!m_activityRecorder.isRecording()) return true;
    // m_activityRecorder stores a pointer to currently recording
    // interval, if activity->intervals.append is called, memory
    // may be moved to another location.
    return activity != m_activityRecorder.activity();
}


void MainWindow::on_splitActivityAction_triggered()
{
    Activity* a = selectedActivity();
    if (a == nullptr)
        return;
    splitActivity(a);
}

void MainWindow::splitActivity(Activity *activity) {
    Q_ASSERT(activity);

    if (!canModifyActivityIntervals(activity) || (m_activityRecorder.isRecording() && m_activityRecorder.activity() == activity)) {
        QMessageBox::critical(this, "Error", "Unable to split this activity right now.");
        return;
    }

    if (activity->intervals.count() < 2) {
        QMessageBox::information(this, "Error", "Activity is represented by a single interval, nothing to split.");
        return;
    }

    QVector<Activity*> newActivities = QVector<Activity*>(activity->intervals.count());
    Q_ASSERT(newActivities.length() == activity->intervals.count()); // @TODO: remove this

    if (!g_app.database()->transaction()) {
        QMessageBox::critical(this, "Error", "Unable to start transaction");
        return;
    }

    for (int i = 0; i < activity->intervals.count(); ++i) {
        Interval interval = activity->intervals[i];
        Activity* newActivity = g_app.m_activityAllocator.allocate();
        newActivity->id          = -1;
        newActivity->startTime   = interval.startTime;
        newActivity->endTime     = interval.endTime;
        newActivity->info        = activity->info;
        newActivity->fieldValues = activity->fieldValues;
        newActivity->intervals.append(interval);

        if (!g_app.database()->saveActivity(newActivity)) {
            goto dberror;
        }
        newActivities[i] = newActivity;
    }

    if (!g_app.database()->deleteActivity(activity->id)) {
        goto dberror;
    }

    if (!g_app.database()->commit()) {
        goto dberror;
    }

    removeActivityFromView(activity);

    for (Activity* a : newActivities) {
        Q_ASSERT(a);
        Q_ASSERT(a->id > 0);
        m_currentViewTimePeriodActivities.append(a);
    }
    m_activityListModel->addActivities(newActivities);
    m_activityVisualizer->update();
    updateVisibleActivitiesDurationLabel();

    // @TODO: sort currentViewTime... and list model

    return;

    dberror:
    g_app.database()->rollback();
    QMessageBox::critical(this, "Error", "Unable to save changes in the database.");
    for (Activity* a : newActivities) {
        if (a) g_app.m_activityAllocator.deallocate(a);
    }
}

qint64 MainWindow::getCurrentDayIndex() const {
    return g_app.currentDaySinceEpochLocal();
}

//void MainWindow::dumpCurrent() {
//    qDebug().nospace() << '[';
//    for (const Activity* a : m_currentViewTimePeriodActivities) {
//        qDebug() << a->id;
//    }
//    qDebug().nospace() << ']';
//}

void MainWindow::on_evalScriptAction_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Choose script file", QString(), "Script Files (*.js);;All files (*.*)");
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {

        QMessageBox::critical(this, "Script Loading Error", QString("Unable to open script \"%1\": \"%2\"")
                              .arg(fileName).arg(file.errorString()));
        return;
    }

    QByteArray text = file.readAll();
    file.close();
\
    if (text.isEmpty()) {
        QMessageBox::critical(this, "Script Loading Error", "Script file is empty or invalid.");
        return;
    }

    QString error;
    QByteArray fileNameUtf8 = fileName.toUtf8();

    PluginManager::EvaluationState state = g_app.m_pluginManager.evaluate(text.constData(), fileNameUtf8.constData(), &error);
    if (state != PluginManager::SuccessfulEvaluation) {
        QString title = state == PluginManager::CompilationError ?
                    QStringLiteral("Script Compilation Error") :
                    QStringLiteral("Script Execution Error");
        QString message = state == PluginManager::CompilationError ?
                    QStringLiteral("Unable to compile script \"%1\":\n%2") :
                    QStringLiteral("Unable to execute script \"%1\":\n%2");
        QMessageBox::critical(this, title, message.arg(fileName).arg(error));
        return;
    }
}

void MainWindow::readAndApplySettings() {
    QSettings s;
    s.beginGroup(QStringLiteral("mainWindow"));
    {
        QByteArray geometry = s.value(QStringLiteral("geometry")).toByteArray();
        if (!geometry.isEmpty()) {
            this->restoreGeometry(geometry);
        }
    }
    {
        QByteArray state = s.value(QStringLiteral("state")).toByteArray();
        if (!state.isEmpty()) {
            this->restoreState(state);
        }
    }
    s.endGroup();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_activityRecorder.isRecording()) {
        m_activityRecorder.stop();
    }

    // @TODO: should care about 'version' of saveState?
    QSettings s;
    s.beginGroup(QStringLiteral("mainWindow"));
    s.setValue(QStringLiteral("geometry"), this->saveGeometry());
    s.setValue(QStringLiteral("state"),    this->saveState());
    s.endGroup();

    event->accept();
}
