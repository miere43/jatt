#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QAbstractListModel>
#include <QItemSelection>
#include <QHash>
#include <QMenu>
#include <QContextMenuEvent>
#include <QShortcut>

#include "application_state.h"
#include "activity_visualizer.h"
#include "activity_list_model.h"
#include "activity_recorder.h"
#include "activity_item_delegate.h"
#include "hotkey.h"

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
    bool canModifyActivityIntervals(Activity* activity) const;

    void handleHotkey(Hotkey* hotkey);
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
    void deleteSelectedActivityTriggered(bool checked);
    void on_joinNextActivityAction_triggered();

    void editActivityFieldDialogFinished(int result);
    void activityMenuItemActionTriggered(bool checked);
    void on_evalScriptAction_triggered();
    void on_splitActivityAction_triggered();

    void changePageLeftShortcutActivated();
    void changePageRightShortcutActivated();
private:
    void closeEvent(QCloseEvent *event) override;
    void readAndApplySettings();

    void splitActivity(Activity* activity);

    void setViewTimePeriod(qint64 startTime, qint64 endTime);
    void setViewDay(qint64 day);
    void showAddActivityDialog();

    bool isTimePeriodInView(qint64 startTime, qint64 endTime) const;
    void updateVisibleActivitiesDurationLabel();
    void updateActivityDurationLabel();

    void addQuickActivityButtons();
    void startQuickActivity(ActivityInfo* info);


//    void setCurrentActivity(Activity* activity);
//    Activity* currentActivity();
//    QModelIndex currentActivityIndex();

    // void dumpCurrent();

    void removeActivityFromView(Activity* activity);

    qint64 getCurrentDayIndex() const;

    Activity* selectedActivity() const;
    QModelIndex selectedActivityIndex() const;
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
    Hotkey* m_recorderHotkey;
    Activity* m_lastActiveActivity = nullptr;

    QMenu m_activityItemMenu;
    QMenu m_activityMenu;

    QShortcut* m_changePageLeftShortcut;
    QShortcut* m_changePageRightShortcut;

    QHash<ActivityInfo*, QMenu*> m_listMenus;

    QMenu* createActivityInfoMenu(ActivityInfo* info);
};

#endif // MAINWINDOW_H
