#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "application_state.h"
#include "common.h"

#include "add_activity_dialog.h"
#include "edit_activity_field_dialog.h"

#include <QtAlgorithms>
#include <QDebug>
#include <QMessageBox>
#include <QBoxLayout>

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

    m_activityVisualizer->setTimelineRenderMode(ActivityVisualizer::Full);

    m_activityMenu.addAction(ui->addActivityAction);

    connect(ui->deleteActivityAction, &QAction::triggered,
            this, &MainWindow::deleteSelectedActivityTriggered);

    g_app.m_pluginManager.initialize();

    g_app.database()->loadActivityInfos();
    QList<ActivityInfo*> list = g_app.database()->activityInfos();
    for (ActivityInfo* info : list) {
        m_listMenus.insert(info, createActivityInfoMenu(info));
    }

    addQuickActivityButtons();
}

inline qint64 activitiesDuration(const QVector<Activity*>& activities) {
    qint64 sum = 0;
    for (const Activity* a : activities) { sum += a->duration(); }
    return sum;
}

QMenu* MainWindow::createActivityInfoMenu(ActivityInfo *info) {
    QMenu* m = new QMenu(this);
    m->addAction(ui->editActivityAction);
    m->addAction(ui->deleteActivityAction);
    m->addSeparator();
    m->addAction(ui->joinNextActivityAction);
    m->addSeparator();
    for (const QString& fieldName : info->fieldNames) {
        QAction* action = m->addAction(QString("Edit \"") + fieldName + "\"");
        // @TODO: this is so eh, maybe there is a better solution?
        action->setProperty("fieldName", QVariant(fieldName));
        connect(action, &QAction::triggered,
                this, &MainWindow::activityMenuItemActionTriggered);
    }
    return m;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_activityListModel;
}

void MainWindow::editActivityFieldDialogFinished(int result) {
    QObject* sender = QObject::sender();
    EditActivityFieldDialog* dialog = qobject_cast<EditActivityFieldDialog*>(sender);
    Q_ASSERT(dialog);

    if (result == QDialog::Accepted) {
        Activity* activity = dialog->activity();
        int i = activity->info->fieldNames.indexOf(dialog->fieldName());
        Q_ASSERT(i != -1);
        Q_ASSERT(activity->info->fieldNames.count() == activity->fieldValues.count());
        activity->fieldValues[i] = dialog->newValue().toString();
        m_activityListModel->dataChangedHint(activity);

        g_app.database()->saveActivity(activity);
    }

    dialog->deleteLater();
}

void MainWindow::activityMenuItemActionTriggered(bool checked) {
    Q_UNUSED(checked);

    QObject* sender = QObject::sender();
    QAction* action = qobject_cast<QAction*>(sender);
    Q_ASSERT(action);

    Activity* activity = selectedActivity();
    Q_ASSERT(activity);
    QString fieldName = action->property("fieldName").value<QString>();

    EditActivityFieldDialog* dialog = new EditActivityFieldDialog(activity, fieldName, this);
    connect(dialog, &EditActivityFieldDialog::finished,
            this, &MainWindow::editActivityFieldDialogFinished);
    dialog->showNormal();
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

void MainWindow::setViewTimePeriod(qint64 startTime, qint64 endTime)
{
    m_currentViewTimePeriodStartTime = startTime;
    m_currentViewTimePeriodEndTime   = endTime;

    m_currentViewTimePeriodActivities.clear();

    Q_ASSERT(g_app.database()->loadActivitiesBetweenStartAndEndTime(&m_currentViewTimePeriodActivities, startTime, endTime, true));

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

    m_activityListModel->clear();
    m_activityListModel->setTimePeriod(startTime, endTime);
    m_activityListModel->addActivities(&m_currentViewTimePeriodActivities);

    m_activityVisualizer->setTimePeriod(startTime, endTime, &m_currentViewTimePeriodActivities);
    ui->timePeriodTotalTimeLabel->setText(createDurationStringFromMsecs(activitiesDuration(m_currentViewTimePeriodActivities)));

}

void MainWindow::setViewDay(qint64 day)
{
    if (day < 0) day = 0;

    qint64 startTime = (day * 86400000LL) - g_app.m_properties.localTimeZoneOffsetFromUtc;
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

//ActivityListItem* MainWindow::selectedActivityListItem() const
//{
//    QModelIndexList selection = ui->activitiesListView->selectionModel()->selection().indexes();
//    if (selection.count() == 0)
//    {
//        return nullptr;
//    }
//    return ((ActivityListItem*)selection.at(0).data(Qt::UserRole).value<void*>());
//}

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
        ui->activityDurationLabel->setText(createDurationStringFromMsecs(currentActivity->duration()));
    }

    if (currentActivity->belongsToTimePeriod(m_currentViewTimePeriodStartTime, m_currentViewTimePeriodEndTime)) {
        ui->timePeriodTotalTimeLabel->setText(createDurationStringFromMsecs(activitiesDuration(m_currentViewTimePeriodActivities)));
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

    // ui->deleteActivityAction->setEnabled(activity != nullptr);

    if (m_activityRecorder.isRecording()) return;

    if (activity == nullptr)
    {
        ui->activityDurationLabel->setText(QStringLiteral("00:00:00"));
        m_activityVisualizer->selectActivity(nullptr);
        qDebug() << "select null";
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

    int index = m_currentViewTimePeriodActivities.indexOf(activity);
    if (index != -1) {
        m_currentViewTimePeriodActivities.remove(index);
        m_activityListModel->removeActivity(activity);
        m_activityVisualizer->update();
    }

    if (activity->id >= 0) {
        g_app.database()->deleteActivity(activity->id);
    }

    g_app.m_activityAllocator.deallocate(activity);
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

    Q_ASSERT(g_app.database()->saveActivity(activity));

    if (activity->belongsToTimePeriod(m_currentViewTimePeriodStartTime, m_currentViewTimePeriodEndTime)) {
        m_activityListModel->addActivity(activity);
    } else {
        setViewDay(getCurrentDayIndex());
    }

    m_activityRecorder.record(activity);
}

void MainWindow::onAppAboutToQuit() {
    if (m_activityRecorder.isRecording())
        m_activityRecorder.stop();
}

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

    int index = m_currentViewTimePeriodActivities.indexOf(next);
    if (index != -1) {
        m_currentViewTimePeriodActivities.remove(index);
        m_activityVisualizer->update();
        m_activityListModel->removeActivity(next);
    }

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

qint64 MainWindow::getCurrentDayIndex() const {
    // @TODO: doesn't work if program was launched at 23:59, but now is 00:00, will still return previous day.
    return g_app.m_currentDaySinceEpochUtc;
}

//void MainWindow::dumpCurrent() {
//    qDebug().nospace() << '[';
//    for (const Activity* a : m_currentViewTimePeriodActivities) {
//        qDebug() << a->id;
//    }
//    qDebug().nospace() << ']';
//}
