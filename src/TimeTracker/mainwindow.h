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
#include "session_item_delegate.h"
#include "recording_item_delegate.h"
#include "recording_dialog.h"
#include "session_visualizer.h"
#include "edit_session_dialog.h"
#include "edit_recording_dialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    // void currentSessionChanged(Session* prev, Session* new);

public slots:
    void createSession();
    void startStopSession();
    void createRecording();

private slots:
    void sessionListViewDoubleClicked(const QModelIndex& index);
    void sessionListViewSelectionChanged(const QItemSelection& index);
    void sessionListViewContextMenuRequested(const QPoint& pos);
    void sessionRecordStateChanged(SessionRecorder* recorder, RecordingState state);

    void recordingListViewContextMenuRequested(const QPoint& pos);

    void editSessionDialogFinished(int result);
    void editRecordingDialogFinished(int result);

    void on_newSessionAction_triggered();
    void on_createRecordingAction_triggered();
    void on_editSessionAction_triggered();

    void recordUpdateTimerTimeout();
    void on_editRecordingAction_triggered();

    void on_editTagsAction_triggered();

private:
    Session* selectedSession();
    Recording* selectedRecording();
    // called from startStopSession();
//    void intervalDialogAccepted();
//    void intervalDialogRejected();
//    RecordingDialog* currentRecordingDialog = nullptr;

    void loadAllSessions();
    void setCurrentSession(Session* session);

    RecordingListModel* getOrCreateRecordingListModelForSession(Session* session);

    SessionListModel sessionListModel;
    //RecordingListModel recordingListModel;

    Session* m_currentSession = nullptr;
    RecordingListModel* m_currentSessionRecordingListModel = nullptr;
    QHash<Session*, RecordingListModel*> m_sessionRecordingListModels;

    Ui::MainWindow *ui;
    SessionVisualizer* vvis;
    SessionItemDelegate sessionItemDelegate;
    RecordingItemDelegate recordingItemDelegate;
    QTimer m_recordUpdateTimer;

    // Shown when context menu requested at empty space.
    QMenu sessionListViewMenu;
    // Shown when context menu requested at item.
    QMenu sessionListViewItemMenu;
    // Shown when context menu requested at item.
    QMenu recordingListViewItemMenu;

    //EditSessionDialog* m_currentEditSessionDialog = nullptr;
};

#endif // MAINWINDOW_H
