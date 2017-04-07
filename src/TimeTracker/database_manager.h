#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

// Debug variable, doesn't commit changes to database when set to 1.
// e.g. uploadSession(...) will not write anything to database, but method will return true.
#define APP_DBMANAGER_NO_COMMIT 0

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QHash>
#include "core_types.h"

class DatabaseManager : public QObject
{
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    bool establishDatabaseConnection();
    bool closeDatabaseConnection();

    bool saveSession(Session* session);
    bool saveRecording(Recording* recording);
    bool saveTag(Tag2* tag);

    Session* loadSession(qint64 sessionId);
    /**
     * @brief Loads recording with specified ID from database. If recording is already loaded, it's pointer is returned.
     * Uses g_app.createRecording() internally if recording is not loaded into application's memory.
     * @param recordingId ID of recording to load from database.
     * @return Pointer to Recording instance or nullptr if error occured.
     */
    Recording* loadRecording(qint64 recordingId);

    bool loadAllSessions(QVector<Session*>* sessions);

    bool loadRecordingTags(Recording* recording, QVector<Tag2*>* tags);
    /**
     * @brief Loads all recordings of specified session into 'recordings' array.
     * @param session
     * @param recordings
     * @return
     */
    bool loadSessionRecordings(Session* session, QVector<Recording*>* recordings);

    bool loadRecordingFromQuery(Recording* recording, QSqlQuery* query);
    bool loadSessionFromQuery(Session* session, QSqlQuery* query);
    bool loadTagFromQuery(Tag2* tag, QSqlQuery* query);

    bool loadAllTags();

    Tag2* loadTagWithId(qint64 tagId);

    /**
     * @brief Returns tag with name 'tagName' from cache or loads it from the database or creates it with such name.
     * @param tagName Name of the tag.
     * @return Pointer to allocated 'Tag' instance with name 'tagName' or nullptr if error occured.
     */
    Tag2* getTagByNameOrCreateIt(const QString& tagName);

    /**
     * @brief Loads tags associated with 'recording' in 'recordingTags' QVector.
     * @param recording Recording, which tags are queried.
     * @param recordingTags Pointer to allocated QVector of Tags, which will receive recording tags.
     * @return True on success, false on failure. 'recordingsTags' may be changed even if this
     * function returns 'false'.
     */
    bool loadTagsAssociatedWithRecording(Recording* recording, QVector<Tag2*>* recordingTags);

    bool associateTagsWithRecording(Recording* recording, QVector<Tag2*>* recordingTags);

    QList<Tag2*> getCachedTags() const;


    bool loadProperties(UserProperties* properties);
    bool saveProperty(QString property, QString value);


    QSqlDatabase m_database;
    QHash<qint64, Recording*> m_recordings;
    QHash<qint64, Session*>   m_sessions;
    QHash<qint64, Tag2*>       m_tags;
};

#endif // DATABASE_MANAGER_H
