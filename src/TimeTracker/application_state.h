#ifndef APPLICATION_STATE_H
#define APPLICATION_STATE_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QString>
#include <QLinkedList>
#include <QVector>
#include "core_types.h"
#include "session_list_model.h"
#include "block_allocator.h"
#include "session_recorder.h"
#include "database_manager.h"
#include "recording_list_model.h"

class ApplicationState : public QObject
{
    Q_OBJECT

private:
    BlockAllocator<Session> m_sessionAllocator;
    BlockAllocator<Recording> m_recordingAllocator;
    BlockAllocator<Tag> m_tagAllocator;
    QVector<Session*> m_sessions;
    QVector<Recording*> m_recordings;
    QVector<Tag*> m_tags;
    QVector<Session*> m_dirtySessions;
    QVector<Recording*> m_dirtyRecordings;
    SessionRecorder m_recorder;
    DatabaseManager m_databaseManager;
public:
    explicit ApplicationState(QObject *parent = 0);

    // @TODO
    // bool initializeSubsystems();

    bool openDatabase(); // @Obsolete
    Session*   createSession();
    Recording* createRecording();
    Tag*       createTag();

    void       freeRecording(Recording* recording);

    void initialize();

    void setShouldUpdate(Session* session);
    void setShouldUpdate(Recording* recording);

    SessionRecorder* recorder();
    DatabaseManager* database();
private slots:
    void appAboutToQuit();
};

//struct DatabaseQueryMaster {
//    // Clears all data from 'tags' and fills it with all tags from database.
//    // Returns number of tags found, -1 if query failed.
//    // May return invalid tags, their ID will be lesser than 0 (tag->id < 0).
//    int queryTags(QSqlDatabase database, QVector<IntervalTag>& tags);
//};

extern ApplicationState g_app;

#endif // APPLICATION_STATE_H
