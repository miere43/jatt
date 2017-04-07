#include "database_manager.h"

#include <QtSql/QSqlError>
#include <QVariant>
#include <QByteArray>

#include "common.h"
#include "application_state.h"

#define APP_STRINGIFY(x) #x
#define APP_LOAD_FROM_QUERY(query, instance, name, type) instance->name = query.value(STRINGIFY(name)).value<type>();

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{

}

bool DatabaseManager::establishDatabaseConnection()
{
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    if (!m_database.isValid())
    {
        qCritical() << "Unable to add SQLite database: " << m_database.lastError().text();
        return false;
    }

    m_database.setDatabaseName("D:/test.s3db"); // @TODO temporary
    if (!m_database.open())
    {
        qCritical() << "Unable to open SQLite database: " << m_database.lastError().text();
        // QSqlDatabase::removeDatabase @TODO is this required?
        return false;
    }

    return true;
}

bool DatabaseManager::closeDatabaseConnection()
{
    if (!m_database.isValid())
    {
        return false;
    }

    if (m_database.isOpen())
    {
        m_database.close();
    }

    return true;
}

bool DatabaseManager::saveSession(Session *session)
{
#if APP_DBMANAGER_NO_COMMIT
    qDebug() << __FUNCTION__;
    return true;
#else
    if (session == nullptr)
    {
        APP_ERRSTREAM << "session is null";
        return false;
    }
    Q_ASSERT(m_database.isOpen());

    QSqlQuery query = QSqlQuery(m_database);
    if (session->id == -1)
    {
        query.prepare("INSERT INTO session(name, note, created, timezone) VALUES (:name, :note, :created, :timezone)");
        query.bindValue(":name", session->name);
        query.bindValue(":note", session->note);
        query.bindValue(":created", QVariant(session->created));
        query.bindValue(":timezone", session->timezone);

        if (query.exec())
        {
            QVariant lastInsertId = query.lastInsertId();
            if (!lastInsertId.isValid())
            {
                APP_ERRSTREAM << "last insert id not supported";
                return false;
            }
            session->id = lastInsertId.value<qint64>();
            m_sessions.insert(session->id, session);
            return true;
        }
        else
        {
            APP_ERRSTREAM << "unable to insert, error:" << query.lastError().text();
            return false;
        }
    }
    else
    {
        query.prepare("UPDATE session SET name = :name, note = :note, created = :created, timezone = :timezone WHERE id = :id");
        query.bindValue(":name", session->name);
        query.bindValue(":note", session->note);
        query.bindValue(":created", QVariant(session->created));
        query.bindValue(":timezone", session->timezone);
        query.bindValue(":id", session->id);

        if (query.exec())
        {
            return true;
        }
        else
        {
            APP_ERRSTREAM << "unable to update, error:" << query.lastError().text();
            return false;
        }
    }
}
#endif

bool DatabaseManager::saveRecording(Recording *recording)
{
#if APP_DBMANAGER_NO_COMMIT
    qDebug() << __FUNCTION__;
    return true;
#else
    Q_ASSERT(recording);
    Q_ASSERT(recording->session);
    Q_ASSERT(recording->session->id > 0);

    if (recording->id == -1)
    {
        QSqlQuery query = QSqlQuery(m_database);
        query.prepare("INSERT INTO recording(session_id, name, note, start_time, end_time, intervals) VALUES (:session_id, :name, :note, :start_time, :end_time, :intervals)");
        query.bindValue(":session_id", recording->session->id);
        query.bindValue(":name", recording->name);
        query.bindValue(":note", recording->note);
        query.bindValue(":start_time", QVariant(recording->startTime));
        query.bindValue(":end_time", QVariant(recording->endTime));

        // @TODO: big endian / little endian stuff
        QByteArray intervals((const char*)recording->intervals.data(), sizeof(Interval) * recording->intervals.count());
        query.bindValue(":intervals", intervals);

        for (int j = 0; j < recording->intervals.count(); ++j)
        {
            qDebug() << "saving recording" << j << "start=" << recording->intervals[j].startTime << "end=" << recording->intervals[j].endTime;
        }

        if (query.exec())
        {
            QVariant lastInsertIdVariant = query.lastInsertId();
            if (!lastInsertIdVariant.isValid())
            {
                APP_ERRSTREAM << "last insert id not supported";
                return false;
            }
            recording->id = lastInsertIdVariant.toInt();
            m_recordings.insert(recording->id, recording);
            return true;
        }
        else
        {
            APP_ERRSTREAM << "unable to insert, error:" << query.lastError().text();
            return false;
        }
    }
    else
    {
        QSqlQuery query = QSqlQuery(m_database);
        query.prepare("UPDATE recording SET session_id = :session_id, name = :name, note = :note, start_time = :start_time, end_time = :end_time, intervals = :intervals WHERE id = :id");
        query.bindValue(":id", recording->id);
        query.bindValue(":session_id", recording->session->id);
        query.bindValue(":name", recording->name);
        query.bindValue(":note", recording->note);
        query.bindValue(":start_time", QVariant(recording->startTime));
        query.bindValue(":end_time", QVariant(recording->endTime));

        // @TODO: big endian / little endian stuff
        QByteArray intervals((const char*)recording->intervals.data(), sizeof(Interval) * recording->intervals.count());
        query.bindValue(":intervals", intervals);

        if (query.exec())
        {
            return true;
        }
        else
        {
            APP_ERRSTREAM << "unable to update, error:" << query.lastError().text();
            return false;
        }
    }
#endif
}

bool DatabaseManager::saveTag(Tag *tag)
{
    Q_ASSERT(tag);

    if (tag->id < 1)
    {
        QSqlQuery query(m_database);
        query.prepare("INSERT INTO tag(name, note, color) VALUES(:name, :note, :color)");
        query.bindValue(":name", tag->name);
        query.bindValue(":note", tag->note);
        query.bindValue(":color", tag->color);

        if (query.exec())
        {
            tag->id = query.lastInsertId().value<qint64>();
            Q_ASSERT(tag->id > 0);
            m_tags.insert(tag->id, tag);
            return true;
        }
        else
        {
            APP_ERRSTREAM << "Unable to insert tag" << tag->id << ":" << tag->name << ", error:" << query.lastError().text();
            return false;
        }
    }
    else
    {
        QSqlQuery query(m_database);
        query.prepare("UPDATE tag SET name = :name, note = :note, color = :color WHERE id = :id");
        query.bindValue(":id", tag->id);
        query.bindValue(":name", tag->name);
        query.bindValue(":note", tag->note);
        query.bindValue(":color", tag->color);

        if (query.exec())
        {
            return true;
        }
        else
        {
            APP_ERRSTREAM << "Unable to update tag" << tag->id << ":" << tag->name << ", error:" << query.lastError().text();
            return false;
        }
    }
}

Session* DatabaseManager::loadSession(qint64 sessionId)
{
    Q_ASSERT(sessionId > 0);

    Session* session = m_sessions.value(sessionId);
    if (session != nullptr)
        return session;

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM session WHERE id = :id");
    query.bindValue(":id", sessionId);

    if (query.exec())
    {
        if (query.next())
        {
            session = g_app.createSession();
            if (loadSessionFromQuery(session, &query))
            {
                return session;
            }
            else
            {
                // @TODO: free session.
                return nullptr;
            }
        }
    }

    return nullptr;
}

Recording* DatabaseManager::loadRecording(qint64 recordingId)
{
    Q_ASSERT(recordingId > 0);

    Recording* recording = m_recordings.value(recordingId);
    if (recording)
        return recording;

    QSqlQuery query(m_database);
    query.prepare("SELECT session_id FROM recording WHERE id = :id");
    query.bindValue(":id", recordingId);

    if (query.exec())
    {
        if (!query.next())
        {
            // No sessions with such session_id.
            APP_ERRSTREAM << "Recording" << recordingId << "references undefined session!";
            return nullptr;
        }

        qint64 sessionId = query.value("session_id").value<qint64>();
        // Loading session will load all it's recordings.
        Q_ASSERT(loadSession(sessionId));

        // Now we look for it in recordings array. Actually we can look it in
        // Session instance, returned from loadSession.
        recording = m_recordings.value(recordingId);
        if (recording)
            return recording;
        return nullptr; // Not found
    }

    // @TODO: report SQL error.
    return nullptr;
}

//    Recording* recording = m_recordings.value(recordingId);
//    if (recording != nullptr)
//        return recording;

//    QSqlQuery query(m_database);
//    query.prepare("SELECT * FROM recording WHERE id = :id");
//    query.bindValue(":id", recordingId);

//    if (query.exec())
//    {
//        // Q_ASSERT(query.size() == 1);

//        if (query.next())
//        {
//            recording = g_app.createRecording();
//            if (loadRecordingFromQuery(recording, &query))
//            {
//                return recording;
//            }
//            else
//            {
//                g_app.freeRecording(recording);
//                return nullptr;
//            }
//        }
//    }

//    // @TODO: report error
//    return nullptr;
//}

bool DatabaseManager::loadSessionRecordings(Session *session, QVector<Recording *> *recordings)
{
    return true;
//    Q_ASSERT(session);
//    Q_ASSERT(recordings);
//    Q_ASSERT(session->id > 0);

//    QSqlQuery query(m_database);
//    query.prepare("SELECT * FROM recording WHERE session_id = :session_id");
//    query.bindValue(":session_id", session->id);

//    if (query.exec())
//    {
//        while (query.next())
//        {
//            qint64 recordingId = query.value("id").value<qint64>();
//            Recording* recording = m_recordings.value(recordingId);
//            if (recording == nullptr)
//            {
//                recording = loadRecording(recordingId);
//                Q_ASSERT(recording);
//            }
//            recordings->append(recording);
//        }
//        return true;
//    }
//    else
//    {
//        APP_ERRSTREAM << "unable to load recordings, error:" << query.lastError().text();
//        return false;
//    }
}

bool DatabaseManager::loadAllSessions(QVector<Session *> *sessions)
{
    Q_ASSERT(sessions);
    QSqlQuery query(m_database);

    if (query.exec("SELECT * FROM session"))
    {
        while (query.next())
        {
            qint64 sessionId = query.value("id").value<qint64>();
            Session* session = m_sessions.value(sessionId);
            if (session == nullptr)
            {
                session = g_app.createSession();
                Q_ASSERT(loadSessionFromQuery(session, &query));
                m_sessions.insert(session->id, session);
            }
            sessions->append(session);
        }
    }

    return true;
}

bool DatabaseManager::loadAllTags()
{
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(1) FROM tag");
    if (!query.exec())
    {
        APP_ERRSTREAM << "Unable get count of tags in 'tag' table.";
        return false;
    }

    qint64 numTags = -1;
    if (query.next())
    {
        numTags = query.value(0).value<qint64>();
        if (numTags == m_tags.size())
        {
            // Assume that all tags are loaded.
            return true;
        }
    }
    else
    {
        APP_ERRSTREAM << "weird stuff";
    }

    //Q_ASSERT(m_tags.size() > numTags); // Upload stuff to DB ASAP.
    query.clear();

    query.prepare("SELECT * FROM tag");
    query.setForwardOnly(true);
    if (!query.exec())
    {
        APP_ERRSTREAM << "Unable to load all tags.";
        return false;
    }
    else
    {
//        QVector<qint64> unloadedTagIds;
//        unloadedTagIds.reserve(clamp(query.count() - m_tags.size(), 0, 8192 / sizeof(qint64)));

        m_tags.reserve(query.size());
        while (query.next())
        {
            qint64 tagId = query.value("id").value<qint64>();
            if (m_tags.contains(tagId))
            {
                continue;
            }
            // @TODO: it's slow to load tags one by one, it's better to batch stuff.
            Tag* tag = loadTagWithId(tagId);
            if (!tag)
            {
                APP_ERRSTREAM << "Unable to load tag" << tagId;
                continue;
            }
            m_tags.insert(tag->id, tag);
        }
    }

    return true;
}

bool DatabaseManager::loadRecordingTags(Recording *recording, QVector<Tag *> *tags)
{
    return true;
//    Q_ASSERT(recording);
//    Q_ASSERT(tags);

//    if (recording->id < 1 && !saveRecording(recording))
//        return false;

//    Q_ASSERT(recording->id > 0);

//    QSqlQuery query(m_database);
//    query.prepare("SELECT * FROM tag WHERE recording_id = :recording_id");
//    query.bindValue(":recording_id", recording->id);

//    if (query.exec())
//    {
//        while (query.next())
//        {
//            qint64 tagId = query.value("id").value<qint64>();
//            Tag* tag = m_tags.value(tagId);
//            if (tag == nullptr)
//            {
//                tag = g_app.createTag();
//                Q_ASSERT(loadTagFromQuery(tag, &query));
//                m_tags.insert(tag->id, tag);
//            }
//            tags->append(tag);
//        }

//        return true;
//    }
//    else
//    {
//        // @TODO: report error
//        return false;
//    }
}

bool DatabaseManager::loadSessionFromQuery(Session *session, QSqlQuery *query)
{
    Q_ASSERT(session);
    Q_ASSERT(query);

    session->id =       query->value("id"      ).value<qint64>();
    session->name =     query->value("name"    ).value<QString>();
    session->note =     query->value("note"    ).value<QString>();
    session->timezone = query->value("timezone").value<qint64>();
    session->created =  query->value("created" ).value<qint64>();

    QSqlQuery recQuery(m_database);
    recQuery.prepare("SELECT * FROM recording WHERE session_id = :session_id");
    recQuery.bindValue(":session_id", session->id);

    if (recQuery.exec())
    {
        while (recQuery.next())
        {
            Recording* recording = g_app.createRecording();
            recording->session = session;
            Q_ASSERT(loadRecordingFromQuery(recording, &recQuery));

            session->recordings.append(recording);
        }
    }
    else
    {
        // @TODO: report error
        return false;
    }

    qDebug().nospace() << "Loaded Session '" << session->name << "' (id " << session->id << ")";
    session->dumpToQDebug();
    return true;
}

Tag* DatabaseManager::getTagByNameOrCreateIt(const QString& tagNameUntrimmed)
{
    QString tagName = tagNameUntrimmed.trimmed();
    if (tagName.isNull() || tagName.isEmpty())
    {
        return nullptr;
    }

    // Find tag in cache, if exists.
    {
        Tag* tag = nullptr;
        for (auto it = m_tags.begin(); it != m_tags.end(); ++it)
        {
            if (it.value()->name == tagName)
            {
                tag = it.value();
                break;
            }
        }

        if (tag != nullptr)
        {
            return tag;
        }
    }

    // If there are no tag in the cache, load it from database.
    {
        QSqlQuery query = QSqlQuery(m_database);
        query.prepare("SELECT * FROM tag WHERE name = :name");
        query.bindValue(":name", tagName);

        if (!query.exec())
        {
            APP_ERRSTREAM << "Unable to execute query, error:" << query.lastError().text();
            return nullptr;
        }

        int querySize = query.size();
        if (querySize > 0)
        {
            if (querySize > 1)
            {
                APP_ERRSTREAM << "Tag with same name" << tagName << "was found in database, something is wrong!";
            }

            if (query.next())
            {
                Tag* newTag = g_app.createTag();
                if (newTag == nullptr)
                {
                    return nullptr;
                }

                if (!loadTagFromQuery(newTag, &query))
                {
                    // @TODO: free 'tag'
                    return nullptr;
                }

                m_tags.insert(newTag->id, newTag);
                return newTag;
            }
        }
    }

    // If there are no tag in the database, create it.
    {
        Tag* tag = g_app.createTag();
        if (tag == nullptr)
        {
            return nullptr;
        }

        tag->name = tagName;
        tag->note = "";
        tag->color = 0;

        if (!saveTag(tag))
        {
            // @TODO: free 'tag'
            return nullptr;
        }

        m_tags.insert(tag->id, tag);

        return tag;
    }

    Q_ASSERT(false); // Unreachable.
    return nullptr;
}

bool DatabaseManager::loadRecordingFromQuery(Recording *recording, QSqlQuery *query)
{
    Q_ASSERT(recording);
    Q_ASSERT(query);

    recording->id = query->value("id").value<qint64>();
    recording->startTime = query->value("start_time").value<qint64>();
    recording->endTime = query->value("end_time").value<qint64>();
    recording->name = query->value("name").value<QString>();
    recording->note = query->value("note").value<QString>();

    QByteArray binaryIntervals = query->value("intervals").toByteArray();
    Q_ASSERT(binaryIntervals.count() % sizeof(Interval) == 0);

    int numIntervals = binaryIntervals.count() / sizeof(Interval);
    qint64* data = (qint64*)binaryIntervals.data();
    binaryIntervals.reserve(numIntervals);
    for (int i = 0; i < numIntervals; ++i)
    {
        recording->intervals.append(Interval {
                                       *(data + (i * 2)),
                                       *(data + (i * 2) + 1)
                                    });
    }

    binaryIntervals.clear();

    qDebug().nospace() << "- Loaded recording " << recording->name << " (id = " << recording->id << ")";
    return true;
}

bool DatabaseManager::loadTagFromQuery(Tag *tag, QSqlQuery *query)
{
    Q_ASSERT(tag);
    Q_ASSERT(query);

    tag->id = query->value("id").value<qint64>();
    tag->name = query->value("name").value<QString>();
    tag->note = query->value("note").value<QString>();
    tag->color = query->value("color").value<qint64>();

    qDebug().nospace() << "- Loaded tag " << tag->name << " (id = " << tag->id << ")";
    return tag;
}

Tag* DatabaseManager::loadTagWithId(qint64 tagId)
{
    Q_ASSERT(tagId > 0);

    Tag* tag = m_tags.value(tagId);
    if (tag != nullptr)
    {
        return tag;
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM tag WHERE id = :id");
    query.bindValue(":id", tagId);

    if (!query.exec())
    {
        APP_ERRSTREAM << "Unable to load tag with ID" << tagId << "from database, error:" << query.lastError().text();
        return nullptr;
    }

    if (query.size() > 1)
    {
        APP_ERRSTREAM << "More than one tag with ID" << tagId << "were found tags table";
    }

    if (query.next())
    {
        tag = g_app.createTag();
        if (tag == nullptr)
        {
            APP_ERRSTREAM << "Unable to allocate tag.";
            return nullptr;
        }

        if (!loadTagFromQuery(tag, &query))
        {
            // @TODO: free tag.
            APP_ERRSTREAM << "Unable to load tag from query, error:" << query.lastError().text();
            return nullptr;
        }

        Q_ASSERT(tagId == tag->id);
        m_tags.insert(tag->id, tag);
    }

    return tag;
}

bool DatabaseManager::loadTagsAssociatedWithRecording(Recording* recording, QVector<Tag*>* recordingTags)
{
    Q_ASSERT(recording);
    Q_ASSERT(recording->id > 0);
    Q_ASSERT(recordingTags);

    QSqlQuery query;
    query.prepare("SELECT tag_id FROM recording_tags WHERE recording_id = :recording_id");
    query.bindValue(":recording_id", recording->id);

    if (!query.exec())
    {
        APP_ERRSTREAM << "Unable to get tags for recording" << recording->id << recording->name << ", error:" << query.lastError().text();
        return false;
    }

    while (query.next())
    {
        qint64 tagId = query.value("tag_id").value<qint64>();
        if (tagId < 1)
        {
            APP_ERRSTREAM << "Invalid tag with ID < 1 was found in recording_tags database table!";
            continue;
        }

        Tag* tag = loadTagWithId(tagId);
        if (tag == nullptr)
        {
            APP_ERRSTREAM << "Unable to load tag with ID =" << tagId << "!";
            continue;
        }

        recordingTags->append(tag);
    }

    return true;
}

bool DatabaseManager::associateTagsWithRecording(Recording* recording, QVector<Tag*>* recordingTags)
{
    Q_ASSERT(recording);
    Q_ASSERT(recordingTags);

    if (recordingTags->count() == 0)
    {
        return true;
    }

    QVariantList recordingIds;
    recordingIds.reserve(recordingTags->count());
    QVariantList tagIds;
    tagIds.reserve(recordingTags->count());

    for (Tag* tag : *recordingTags)
    {
        if (tag->id < 0)
        {
            if (!this->saveTag(tag))
            {
                APP_ERRSTREAM << "unable to save tag" << tag->name;
                return false;
            }
            Q_ASSERT(tag->id > 0);
        }
        recordingIds.append(recording->id);
        tagIds.append(tag->id);
    }

    // These things would not insert already existing associations because table has
    // 'UNIQUE(recording_id, tag_id) ON CONFLICT IGNORE' clause.
    QSqlQuery query;
    query.prepare("INSERT INTO recording_tags(recording_id, tag_id) VALUES(?, ?)");
    query.addBindValue(recordingIds);
    query.addBindValue(tagIds);

    if (!query.execBatch())
    {
        APP_ERRSTREAM << "Unable to insert tags in recording_tags";
        return false;
    }

    return true;
}

QList<Tag*> DatabaseManager::getCachedTags() const
{
    return m_tags.values();
}
