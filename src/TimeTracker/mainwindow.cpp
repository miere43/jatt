#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "application_state.h"
#include "common.h"

#include "add_activity_dialog.h"

#include <QDebug>
#include <QMessageBox>
#include <QBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_activityListModel(new ActivityListModel)
    , m_activityVisualizer(new ActivityVisualizer)
{
    ui->setupUi(this);
    ui->activitiesListView->setModel(m_activityListModel);

    qint64 startTime = g_app.m_currentDaySinceEpochUtc * 86400000LL;
    qint64 endTime = startTime + 86400000LL;

    setViewDay(g_app.m_currentDaySinceEpochUtc - g_app.m_properties.firstActivityDayUtc);
    setViewTimePeriod(startTime, endTime);

    ui->verticalLayout_4->addWidget(m_activityVisualizer);

    connect(&m_activityRecorder, &ActivityRecorder::recordEvent,
            this, &MainWindow::activityRecorderRecordEvent);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_activityListModel;
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
        if (activity->startTime > m_currentViewTimePeriodStartTime || activity->endTime < m_currentViewTimePeriodEndTime)
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

    Q_ASSERT(g_app.database()->loadActivitiesBetweenStartAndEndTime(&m_currentViewTimePeriodActivities, startTime, endTime));

    m_activityListModel->clear();
    m_activityListModel->addActivities(&m_currentViewTimePeriodActivities);

    m_activityVisualizer->setTimePeriod(startTime, endTime, &m_currentViewTimePeriodActivities);
}

void MainWindow::setViewDay(qint64 day)
{
    if (day < 0) day = 0;

    qint64 startTime = (day + g_app.m_properties.firstActivityDayUtc) * 86400000LL + g_app.m_properties.localTimeZoneOffsetFromUtc;
    qint64 endTime = startTime + 86400000LL + g_app.m_properties.localTimeZoneOffsetFromUtc;
    setViewTimePeriod(startTime, endTime);

    ui->dayLabel->setText("Day <b>" + QString::number(day + 1) + "</b>");
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
        Q_ASSERT(activity);

        m_activityRecorder.record(activity);
    }
}

Activity* MainWindow::selectedActivity()
{
    QModelIndexList selection = ui->activitiesListView->selectionModel()->selection().indexes();
    if (selection.count() == 0)
    {
        return nullptr;
    }
    return (Activity*)selection.at(0).data(Qt::UserRole).value<void*>();
}

void MainWindow::activityRecorderRecordEvent(ActivityRecorderEvent event)
{
    qDebug() << m_activityRecorder.activity()->endTime;

    if (event == ActivityRecorderEvent::RecordingStarted)
    {
        ui->startActivityButton->setText("Stop Activity");
    }
    else if (event == ActivityRecorderEvent::RecordingStopped)
    {
        ui->startActivityButton->setText("Start Activity");
        Activity* activity = m_activityRecorder.activity();
        g_app.database()->saveActivity(activity);
    }

    m_activityVisualizer->update();
}
