#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QAbstractListModel>
#include <QItemSelection>
#include <QHash>
#include <QMenu>
#include <QContextMenuEvent>

#include "application_state.h"
#include "activity_visualizer.h"
#include "activity_list_model.h"
#include "activity_recorder.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setViewTimePeriod(qint64 startTime, qint64 endTime);
    void setViewDay(qint64 day);

private slots:
    void on_addActivityAction_triggered();
    void addActivityDialogFinished(int result);

    void on_nextDayButton_clicked();
    void on_prevDayButton_clicked();
    void on_startActivityButton_clicked();

    void activityRecorderRecordEvent(ActivityRecorderEvent event);
private:
    Activity* selectedActivity();

    Ui::MainWindow *ui = nullptr;
    ActivityListModel* m_activityListModel = nullptr;
    ActivityVisualizer* m_activityVisualizer = nullptr;
    QVector<Activity*> m_currentViewTimePeriodActivities;
    // Starts with 0!
    qint64 m_viewDay = -1;

    qint64 m_currentViewTimePeriodStartTime = -1;
    qint64 m_currentViewTimePeriodEndTime = -1;

    ActivityRecorder m_activityRecorder;
};

#endif // MAINWINDOW_H
