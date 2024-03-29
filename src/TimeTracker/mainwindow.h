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
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    const QVector<Activity*>& currentActivities() const;
    bool canModifyActivityIntervals(const Activity* activity) const;

    void handleHotkey(Hotkey* hotkey);

    void setViewDay(qint64 day);
    void selectActivityById(qint64 id);
private slots:
    void on_addActivityAction_triggered();
    void addActivityDialogFinished(int result);

    void selectedActivityChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void activitiesListViewMenuRequested(const QPoint &pos);
    void activitiesListViewActivated(const QModelIndex& index);

    void on_nextDayButton_clicked();
    void on_prevDayButton_clicked();
    void on_startActivityButton_clicked();

    void activityRecorderRecordEvent(ActivityRecorderEvent event);

    void startQuickActivityButtonClicked();
    void quickActivityButtonRightClicked();

    void editActivityFieldDialogFinished(int result);
    void activityMenuItemActionTriggered(bool checked);
    void on_splitActivityAction_triggered();
    void on_activityFillBetweenAction_triggered();

    void editSelectedActivityShortcutActivated();
    void changePageLeftShortcutActivated();
    void changePageRightShortcutActivated();
    void on_openSettingsAction_triggered();
    void on_statisticsAction_triggered();

    void on_editActivityAction_triggered();
    void on_deleteActivityAction_triggered();
    void on_joinNextActivityAction_triggered();
    void on_openActivityBrowserAction_triggered();

    void activityVisualizerMenuRequested(const QPoint & pos);
    void visualizerCreateActivityFromSelection(bool checked);
    void on_enableHotkeyAction_triggered();

private:
    void closeEvent(QCloseEvent *event) override;
    void readAndApplySettings();

    void splitActivity(Activity* activity);
    void showEditActivityFieldDialog(Activity* activity);

    void setViewTimePeriod(qint64 startTime, qint64 endTime);
    void showAddActivityDialog();

    bool isTimePeriodInView(qint64 startTime, qint64 endTime) const;
    void updateVisibleActivitiesDurationLabel();
    void updateActivityDurationLabel();

    void loadQuickActivityVisibilityMenu();
    void addQuickActivityButtons();
    void startQuickActivity(ActivityCategory * category);

    void deleteActivity(Activity* activity);
    void joinActivities(Activity* targetActivity, QVector<Activity*> activitiesToJoin);

    void updateWindowIcon(ActivityCategory * category);

    /**
     * @brief Enables or disabled 'Continue Activity' hotkey.
     */
    void setHotkeyEnabled(bool enabled);
    bool m_hotkeyEnabled = false;

//    void setCurrentActivity(Activity* activity);
//    Activity* currentActivity();
//    QModelIndex currentActivityIndex();

    // void dumpCurrent();

    void removeActivityFromView(Activity* activity);

    qint64 getCurrentDayIndex() const;

    Activity * selectedActivity() const;
    QModelIndex selectedActivityIndex() const;
    // ActivityListItem* selectedActivityListItem() const;

    Ui::MainWindow * ui = nullptr;
    ActivityListModel * m_activityListModel = nullptr; // bunch of stuff relies on this variable, don't touch.
    ActivityVisualizer * m_activityVisualizer = nullptr;
    ActivityItemDelegate * m_activityItemDelegate = nullptr;
    QVector<Activity *> m_currentViewTimePeriodActivities;
    // Starts with 0!
    qint64 m_viewDay = -1;

    qint64 m_currentViewTimePeriodStartTime = -1;
    qint64 m_currentViewTimePeriodEndTime = -1;

    ActivityRecorder m_activityRecorder;
    Hotkey * m_recorderHotkey = nullptr;
    Activity * m_lastActiveActivity = nullptr;

    QMenu m_visualizerMenu;
    QAction * m_visualizerCreateActivityFromSelection = nullptr;

    QMenu m_activityItemMenu;
    Activity * m_activityItemMenu_activity = nullptr;
    QMenu m_activityMenu;

    QMenu m_quickActivityMenu;

    QShortcut* m_changePageLeftShortcut;
    QShortcut* m_changePageRightShortcut;
    QShortcut* m_editSelectedActivityShortcut;
};

#endif // MAINWINDOW_H
