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
#include "activity_item_delegate.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    const QVector<Activity*>& currentActivities() const;
    void onAppAboutToQuit();
private slots:
    void on_addActivityAction_triggered();
    void addActivityDialogFinished(int result);

    void selectedActivityChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void activitiesListViewMenuRequested(const QPoint &pos);

    void on_nextDayButton_clicked();
    void on_prevDayButton_clicked();
    void on_startActivityButton_clicked();

    void activityRecorderRecordEvent(ActivityRecorderEvent event);

    void startQuickActivityButtonClicked();
    void deleteSelectedActivityIntervalTriggered(bool checked);
    void on_joinNextActivityAction_triggered();

private:
    void setViewTimePeriod(qint64 startTime, qint64 endTime);
    void setViewDay(qint64 day);
    void showAddActivityDialog();

    void addQuickActivityButtons();
    void startQuickActivity(ActivityInfo* info);

    Activity* selectedActivity() const;
    // ActivityListItem* selectedActivityListItem() const;

    Ui::MainWindow *ui = nullptr;
    ActivityListModel* m_activityListModel = nullptr; // bunch of stuff relies on this variable, don't touch.
    ActivityVisualizer* m_activityVisualizer = nullptr;
    ActivityItemDelegate* m_activityItemDelegate = nullptr;
    QVector<Activity*> m_currentViewTimePeriodActivities;
    // Starts with 0!
    qint64 m_viewDay = -1;

    qint64 m_currentViewTimePeriodStartTime = -1;
    qint64 m_currentViewTimePeriodEndTime = -1;

    ActivityRecorder m_activityRecorder;

    QMenu m_activityItemMenu;
    QMenu m_activityMenu;
};

#endif // MAINWINDOW_H
