#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "application_state.h"
#include "common.h"
#include "recording_dialog.h"

#include <QDebug>
#include <QMessageBox>
#include <QBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , sessionItemDelegate()
    , recordingItemDelegate(g_app.recorder())
{
    ui->setupUi(this);

    // sessionListViewMenu setup
    sessionListViewMenu.addAction(ui->newSessionAction);

    // sessionListViewItemMenu setup
    sessionListViewItemMenu.addAction(ui->editSessionAction);

    // recordingListViewItemMenu setup
    recordingListViewItemMenu.addAction(ui->editRecordingAction);

    // ui->sessionListView setup
    ui->sessionListView->setModel(&sessionListModel);
    ui->sessionListView->setItemDelegate(&sessionItemDelegate);
    ui->sessionListView->setContextMenuPolicy(Qt::CustomContextMenu);

    // ui->recordingListView setup
    ui->recordingListView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->sessionListView, &QListView::doubleClicked,
            this, &MainWindow::sessionListViewDoubleClicked);
    connect(ui->sessionListView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::sessionListViewSelectionChanged);
    connect(ui->sessionListView, &QListView::customContextMenuRequested,
            this, &MainWindow::sessionListViewContextMenuRequested);
    //
    connect(g_app.recorder(), &SessionRecorder::recordStateChanged,
            this, &MainWindow::sessionRecordStateChanged);
    //
    connect(ui->recordingListView, &QListView::customContextMenuRequested,
            this, &MainWindow::recordingListViewContextMenuRequested);

    static int SESSION_UI_UPDATE_MS = 50;
    m_recordUpdateTimer.setSingleShot(false);
    m_recordUpdateTimer.setInterval(SESSION_UI_UPDATE_MS);

    connect(&m_recordUpdateTimer, &QTimer::timeout,
            this, &MainWindow::recordUpdateTimerTimeout);

    vvis = new SessionVisualizer();
    //vvis->setRecordings(&recordingListModel.m_recordings);
    ui->verticalLayout_4->insertWidget(0, vvis);

    // @TODO: you were about to add menu to sessionListView here.
    // http://doc.qt.io/qt-5.8/qwidget.html#contextMenuPolicy-prop

    loadAllSessions();
}

void MainWindow::sessionListViewContextMenuRequested(const QPoint& pos)
{
    QModelIndex selectedItem = ui->sessionListView->indexAt(pos);
    QPoint globalPos = ui->sessionListView->mapToGlobal(pos);
    if (selectedItem.isValid())
    {
        // Item selected.
        sessionListViewItemMenu.popup(globalPos);
    }
    else
    {
        // Empty space.
        sessionListViewMenu.popup(globalPos);
    }
}

void MainWindow::recordingListViewContextMenuRequested(const QPoint& pos)
{
    QModelIndex selectedItem = ui->recordingListView->indexAt(pos);
    QPoint globalPos = ui->recordingListView->mapToGlobal(pos);
    if (selectedItem.isValid())
    {
        recordingListViewItemMenu.popup(globalPos);
    }
    else
    {
        // @TODO: Not implemented yet for empty space.
    }
}

void MainWindow::setCurrentSession(Session *session)
{
    if (m_currentSession == session)
    {
        return;
    }

    m_currentSession = session;
    if (m_currentSession == nullptr)
    {
        m_currentSessionRecordingListModel = nullptr;
        vvis->setRecordings(nullptr);
    }
    else
    {
        m_currentSessionRecordingListModel = getOrCreateRecordingListModelForSession(m_currentSession);
        vvis->setRecordings(&m_currentSessionRecordingListModel->m_recordings);
    }

    ui->recordingListView->setModel(m_currentSessionRecordingListModel);
    ui->recordingListView->setItemDelegate(&recordingItemDelegate);

    ui->currentSessionLabel->setText(m_currentSession ? "Current Session: " + m_currentSession->name : "Current Session: <none>");
    // @TODO: update UI.
}

void MainWindow::sessionListViewDoubleClicked(const QModelIndex &index)
{
    if (g_app.recorder()->isRecording())
    {
        QMessageBox::critical(this, "Error", "Unable to change current session: already recording.");
        return;
    }

    QVariant sessionVariant = sessionListModel.data(index, Qt::UserRole);
    Session* session = nullptr;
    if (!sessionVariant.isValid() || (session = (Session*)sessionVariant.value<void*>()) == nullptr)
    {
        APP_ERRSTREAM << "unable to get selected session!";
        return;
    }

    setCurrentSession(session);
}

RecordingListModel* MainWindow::getOrCreateRecordingListModelForSession(Session *session)
{
    RecordingListModel* model = m_sessionRecordingListModels.value(session);
    if (model == nullptr)
    {
        model = new RecordingListModel();
        m_sessionRecordingListModels.insert(session, model);
    }
    return model;
}

void MainWindow::loadAllSessions()
{
    QVector<Session*> sessions;
    if (!g_app.database()->loadAllSessions(&sessions))
    {
        QMessageBox::critical(this, "Error", "Unable to load all sessions");
        return;
    }

    for (int i = 0; i < sessions.count(); ++i)
    {
        Session* session = sessions[i];

        sessionListModel.appendSession(session);
        RecordingListModel* recordingListModel = getOrCreateRecordingListModelForSession(session);
        for (int j = 0; j < session->recordings.count(); ++j)
        {
            recordingListModel->appendRecording(session->recordings[j]);
            // set should update? @TODO
        }
        // set should update? @TODO
    }

    return;
}

void MainWindow::createSession()
{
    Session* session = g_app.createSession();
    if (session == nullptr)
    {
        QMessageBox::critical(this, "Error", "Unable to create new session.");
        return;
    }

    session->name = tr("New Session");
    sessionListModel.appendSession(session);
    g_app.setShouldUpdate(session);
}

Session* MainWindow::selectedSession()
{
    QModelIndexList selection = ui->sessionListView->selectionModel()->selectedIndexes();
    if (selection.count() == 0)
    {
        return nullptr;
    }
    else if (selection.count() > 1)
    {
        APP_ERRSTREAM << "only one session can be selected at once.";
    }

    // @TODO: Will this work when list is sorted?
    QVariant sessionVariant = sessionListModel.data(selection.at(0), Qt::UserRole);
    Session* session = nullptr;
    if (!sessionVariant.isValid() || (session = (Session*)sessionVariant.value<void*>()) == nullptr)
    {
        APP_ERRSTREAM << "unable to get selected session!";
        return nullptr;
    }

    return session;
}

Recording* MainWindow::selectedRecording()
{
    if (m_currentSession == nullptr)
    {
        return nullptr;
    }

    QModelIndexList selection = ui->recordingListView->selectionModel()->selectedIndexes();
    if (selection.count() == 0)
    {
        return nullptr;
    }
    else if (selection.count() > 1)
    {
        APP_ERRSTREAM << "only one recording can be selected at once.";
    }


    // @TODO: Will this work when list is sorted?
    QVariant recordingVariant = m_currentSessionRecordingListModel->data(selection.at(0), Qt::UserRole);
    Recording* recording = nullptr;
    if (!recordingVariant.isValid() || (recording = (Recording*)recordingVariant.value<void*>()) == nullptr)
    {
        APP_ERRSTREAM << "unable to get selected recording!";
        return nullptr;
    }

    return recording;
}

void MainWindow::createRecording()
{
    if (m_currentSession == nullptr)
    {
        QMessageBox::critical(this, "Error", "No enabled session.");
        return;
    }

    // @TODO: show dialog
    Recording* recording = g_app.createRecording();
    Q_ASSERT(recording);
    recording->name = "Dummy Recording";
    recording->session = m_currentSession;

    m_currentSession->recordings.append(recording);
    m_currentSessionRecordingListModel->appendRecording(recording);
    g_app.setShouldUpdate(recording);
}

// @TODO: we are starting recordings, not sessions, rename.
void MainWindow::startStopSession()
{
    if (g_app.recorder()->isRecording())
    {
        Recording* recording = g_app.recorder()->recording();
        g_app.recorder()->stop();
        Q_ASSERT(recording);
        Q_ASSERT(m_currentSessionRecordingListModel);
        m_currentSessionRecordingListModel->sendDataChangedHint(recording);
        return;
    }
    else
    {
        Recording* selectedRecording = this->selectedRecording();
        if (!selectedRecording)
        {
            QMessageBox::critical(this, "Error", "Recording is not selected.");
            return;
        }

        if (!g_app.recorder()->record(selectedRecording->session, selectedRecording))
        {
            QMessageBox::critical(this, "Error", "Unable to start recording.");
            // @TODO: delete Recording
            return;
        }

        Q_ASSERT(m_currentSessionRecordingListModel);
        m_currentSessionRecordingListModel->sendDataChangedHint(selectedRecording);
        //g_app.setShouldUpdate(selectedRecording);
    }
}

void MainWindow::sessionRecordStateChanged(SessionRecorder* recorder, RecordingState state)
{
    Q_UNUSED(recorder);

    if (state == RecordingState::Started)
    {
        ui->startStopSessionButton->setText("Stop Recording");
        if (!m_recordUpdateTimer.isActive())
        {
            m_recordUpdateTimer.start();
        }
    }
    else if (state == RecordingState::Finished)
    {
        ui->startStopSessionButton->setText("Start Recording");
        if (m_recordUpdateTimer.isActive())
        {
            m_recordUpdateTimer.stop();
        }

//        recorder->interval()->name += " 123";
//        g_app.intervalListModel.sendDataChangedHint(recorder->interval());
    }
    else
    {
        APP_ERRSTREAM << "unsupported RecordingState.";
    }
}

void MainWindow::sessionListViewSelectionChanged(const QItemSelection &selection)
{
    QModelIndexList indexes = selection.indexes();
    if (indexes.count() == 0)
    {
        Q_ASSERT(!g_app.recorder()->isRecording());
        ui->startStopSessionButton->setEnabled(false);
        //setCurrentSession(selectedSession());
    }
    else if (indexes.count() == 1)
    {
        ui->startStopSessionButton->setEnabled(true);
        //setCurrentSession(selectedSession());
    }
    else
    {
        APP_ERRSTREAM << "only one session should be selected at once.";
    }
}

void MainWindow::recordUpdateTimerTimeout()
{
    if (!g_app.recorder()->isRecording())
    {
        APP_ERRSTREAM << "recordUpdateTimer is functional for some reason, timer should be stopped by now.";
        return;
    }

    qint64 msecs = g_app.recorder()->msecsPassed();
    ui->sessionTimeLabel->setText(createDurationStringFromMsecs(msecs));
    vvis->update();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_newSessionAction_triggered()
{
    this->createSession();
}

void MainWindow::on_createRecordingAction_triggered()
{
    this->createRecording();
}

void MainWindow::on_editSessionAction_triggered()
{
    Session* session = this->selectedSession();
    Q_ASSERT(session);

    EditSessionDialog* dialog = new EditSessionDialog(this);
    dialog->setSession(session);

    connect(dialog, &EditSessionDialog::finished,
            this, &MainWindow::editSessionDialogFinished);

    dialog->exec(); // this is modal, probably we need modeless.
}

void MainWindow::editSessionDialogFinished(int result)
{
    QObject* sender = QObject::sender();
    EditSessionDialog* dialog = qobject_cast<EditSessionDialog*>(sender);
    Q_ASSERT(dialog);

    if (result == QDialog::Accepted)
    {
        // @TODO: no validation was performed, check for weird input
        Session* dialogTargetSession = dialog->session();
        Q_ASSERT(dialogTargetSession);
        dialogTargetSession->name = dialog->sessionName();
        dialogTargetSession->note = dialog->sessionNote();
        g_app.database()->saveSession(dialogTargetSession);
        qDebug() << "dialog succeeded.";
    }
    else
    {
        // rejected
    }

    dialog->deleteLater(); // we are in event loop, should not delete object here.
}

void MainWindow::on_editRecordingAction_triggered()
{
    Recording* recording = this->selectedRecording();
    Q_ASSERT(recording);

    EditRecordingDialog* dialog = new EditRecordingDialog(this);
    dialog->setRecording(recording);

    connect(dialog, &EditRecordingDialog::finished,
            this, &MainWindow::editRecordingDialogFinished);

    dialog->exec(); // this is modal, probably we need modeless.
}

void MainWindow::editRecordingDialogFinished(int result)
{
    QObject* sender = QObject::sender();
    EditRecordingDialog* dialog = qobject_cast<EditRecordingDialog*>(sender);
    Q_ASSERT(dialog);

    if (result != QDialog::Accepted)
    {
        // rejected
        goto skipLabel;
    }

    if (!dialog->isValidationPassed())
    {
        QMessageBox::critical(this, "Edit Recording", "Tags are not valid!");
        goto skipLabel;
    }

    Recording* dialogTargetRecording = dialog->recording();
    Q_ASSERT(dialogTargetRecording);
    dialogTargetRecording->name = dialog->recordingName();
    dialogTargetRecording->note = dialog->recordingNote();
    QVector<Tag*>* recordingTags = dialog->recordingTags();
    dialogTargetRecording->tags = *recordingTags;

    if (!g_app.database()->saveRecording(dialogTargetRecording))
    {
        QMessageBox::critical(this, "Edit Recording", "Unable to save your changes for this recording!");
    }
    else if (!g_app.database()->associateTagsWithRecording(dialogTargetRecording, recordingTags))
    {
        QMessageBox::critical(this, "Edit Recording", "Unable to save recording tags!");
    }

    skipLabel:
    dialog->deleteLater(); // we are in event loop, should not delete object here.
}
