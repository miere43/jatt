#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "application_state.h"
#include "common.h"

#include "add_activity_dialog.h"

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
    ui->setupUi(this);
    ui->activitiesListView->setModel(m_activityListModel);
    ui->activitiesListView->setItemDelegate(m_activityItemDelegate);
    ui->activitiesListView->setContextMenuPolicy(Qt::CustomContextMenu);

    setViewDay(g_app.m_currentDaySinceEpochUtc - g_app.m_properties.firstActivityDayUtc);

    ui->verticalLayout_4->addWidget(m_activityVisualizer);

    connect(&m_activityRecorder, &ActivityRecorder::recordEvent,
            this, &MainWindow::activityRecorderRecordEvent);
    connect(ui->activitiesListView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::selectedActivityChanged);
    connect(ui->activitiesListView, &QListView::customContextMenuRequested,
            this, &MainWindow::activitiesListViewMenuRequested);

    m_activityVisualizer->setTimelineRenderMode(ActivityVisualizer::Full);

    m_activityMenu.addAction(ui->editActivityAction);
    m_activityMenu.addAction(ui->deleteActivityIntervalAction);

    connect(ui->deleteActivityIntervalAction, &QAction::triggered,
            this, &MainWindow::deleteSelectedActivityIntervalTriggered);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_activityListModel;
}

void MainWindow::activitiesListViewMenuRequested(const QPoint &pos)
{
    if (ui->activitiesListView->indexAt(pos).isValid())
        m_activityMenu.exec(ui->activitiesListView->mapToGlobal(pos));
}

void MainWindow::on_addActivityAction_triggered()
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
        if (activity->hasIntervalsBetweenTime(m_currentViewTimePeriodStartTime, m_currentViewTimePeriodEndTime))
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

        if (currentActivity->hasIntervalsBetweenTime(startTime, endTime)) {
            if (!m_currentViewTimePeriodActivities.contains(currentActivity)) {
                qDebug() << "added current activity " << currentActivity->fieldValues[0];
                m_currentViewTimePeriodActivities.append(currentActivity);
            }
        }
    }

    m_activityListModel->clear();
    m_activityListModel->setTimePeriod(startTime, endTime);
    m_activityListModel->addActivities(&m_currentViewTimePeriodActivities);

    m_activityVisualizer->setTimePeriod(startTime, endTime, &m_currentViewTimePeriodActivities);
}

void MainWindow::setViewDay(qint64 day)
{
    if (day < 0) day = 0;

    qint64 startTime = (day + g_app.m_properties.firstActivityDayUtc) * 86400000LL - g_app.m_properties.localTimeZoneOffsetFromUtc;
    qint64 endTime = startTime + 86400000LL;
    setViewTimePeriod(startTime, endTime);

    ui->dayLabel->setText("Day <b>" + QString::number(day + 1) + "</b>, " + QDateTime::fromMSecsSinceEpoch(startTime, Qt::LocalTime).toString("MMM dd"));
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

ActivityListItem* MainWindow::selectedActivityListItem() const
{
    QModelIndexList selection = ui->activitiesListView->selectionModel()->selection().indexes();
    if (selection.count() == 0)
    {
        return nullptr;
    }
    return ((ActivityListItem*)selection.at(0).data(Qt::UserRole).value<void*>());
}

Activity* MainWindow::selectedActivity() const
{
    return selectedActivityListItem()->activity;
}

void MainWindow::activityRecorderRecordEvent(ActivityRecorderEvent event)
{
    // qDebug() << m_activityRecorder.activity()->endTime;

    Activity* currentActivity = m_activityRecorder.activity();
    // Interval* currentInterval = m_activityRecorder.interval();

    if (event == ActivityRecorderEvent::RecordingStarted)
    {
        ui->startActivityButton->setText("Stop Activity");
        m_activityListModel->addActivityInterval(currentActivity, m_activityRecorder.interval());
    }
    else if (event == ActivityRecorderEvent::RecordingStopped)
    {
        g_app.database()->saveActivity(currentActivity);

        ui->startActivityButton->setText("Start Activity");
        // ui->activityDurationLabel->setText(QStringLiteral("00:00:00"));
    }

    if (event == ActivityRecorderEvent::RecordingStarted
        || event == ActivityRecorderEvent::SyncTimer)
    {
        Q_ASSERT(currentActivity);
        ui->activityDurationLabel->setText(createDurationStringFromMsecs(currentActivity->duration()));
    }

    m_activityVisualizer->update();
}

void MainWindow::selectedActivityChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);

    ActivityListItem* listItem = nullptr;
    QModelIndexList indexes = selected.indexes();
    if (indexes.count() > 0)
    {
        const QModelIndex& index = indexes.at(0);
        if (index.isValid())
            listItem = static_cast<ActivityListItem*>(index.data(Qt::UserRole).value<void*>());
    }

    if (m_activityRecorder.isRecording()) return;

    if (listItem == nullptr)
    {
        ui->activityDurationLabel->setText(QStringLiteral("00:00:00"));
        m_activityVisualizer->selectInterval(nullptr);
        qDebug() << "select null";
    }
    else
    {
        ui->activityDurationLabel->setText(createDurationStringFromMsecs(listItem->activity->duration()));
        m_activityVisualizer->selectInterval(listItem->interval);
    }

}

void MainWindow::deleteSelectedActivityIntervalTriggered(bool checked)
{
    Q_UNUSED(checked);
    QModelIndexList selection = ui->activitiesListView->selectionModel()->selection().indexes();
    if (selection.count() == 0 ) {
        APP_ERRSTREAM << "nothing to delete, this action should be disabled.";
        return;
    }

    const QModelIndex& selectionIndex = selection.at(0);
    if (!selectionIndex.isValid()) {
        APP_ERRSTREAM << "selectionIndex is invalid";
        return;
    }

    ActivityListItem* listItem = (ActivityListItem*)selectionIndex.data(Qt::UserRole).value<void*>();
    Q_ASSERT(listItem);
    Q_ASSERT(listItem->activity);

    if (listItem->interval == nullptr) {
        if (0 != listItem->activity->intervals.count()) {
            APP_ERRSTREAM << "invalid interval";
            Q_ASSERT(false);
            return;
        }

        // If activity doesn't have any intervals, just delete activity.
        Q_ASSERT(g_app.database()->deleteActivity(listItem->activity->id));

        m_activityListModel->removeActivity(listItem->activity);
        g_app.m_activityAllocator.deallocate(listItem->activity);
    } else {
        // If activity has interval, remove it.
        int intervalPos = -1;
        for (int i = 0; i < listItem->activity->intervals.count(); ++i) {
            Interval* interval = listItem->activity->intervals.data() + i;
            if (interval == listItem->interval) {
                intervalPos = i;
                break;
            }
        }

        if (intervalPos == -1) {
            APP_ERRSTREAM << "something is wrong";
            return;
        }

        if (m_activityRecorder.interval() == listItem->interval) {
            QMessageBox::critical(this, "Error", "You cannot delete currenly recording interval.");
            return;
        }

        listItem->activity->intervals.remove(intervalPos);
        ui->activitiesListView->model()->removeRow(selectionIndex.row());

        if (!g_app.database()->saveActivity(listItem->activity)) {
            QMessageBox::critical(this, "Error", "Error while trying to update activity.");
            APP_ERRSTREAM << "error removing interval";
            return;
        }
    }
}
