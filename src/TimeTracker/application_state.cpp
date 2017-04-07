#include "application_state.h"

#include <QDebug>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QVariant>
#include <QCoreApplication>
#include "common.h"
#include "date_time.h"

ApplicationState::ApplicationState(QObject *parent)
    : QObject(parent)
    , m_recorder()
    , m_databaseManager()
    , m_sessionAllocator()
{

}

void ApplicationState::initialize()
{
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
            this, &ApplicationState::appAboutToQuit);

    m_properties.firstActivityDay = -1;
    Q_ASSERT(database()->loadProperties(&m_properties));
    qDebug() << "firstActivityDay =" << m_properties.firstActivityDay;

    if (m_properties.firstActivityDay == -1)
    {
        m_properties.firstActivityDay = getCurrentDateTime().completeDaysSinceEpoch();
        qDebug() << "firstActivityDay property is missing, setting it to " << m_properties.firstActivityDay;
        Q_ASSERT(database()->saveProperty("firstActivityDay", QString::number(m_properties.firstActivityDay)));
    }
}

SessionRecorder* ApplicationState::recorder()
{
    return &m_recorder;
}

Session* ApplicationState::createSession()
{
    Session* session = m_sessionAllocator.allocate();
    if (session == nullptr) return nullptr;
    session->id = -1;
    session->timezone = QDateTime::currentDateTime().offsetFromUtc();
    session->created = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qDebug() << session->id << "created at" << session->created;
    return session;
}

Recording* ApplicationState::createRecording()
{
    Recording* recording = m_recordingAllocator.allocate();
    if (recording == nullptr) return nullptr;
    recording->id = -1;
    m_recordings.append(recording);
    return recording;
}

Tag2* ApplicationState::createTag()
{
    Tag2* tag = m_tagAllocator.allocate();
    if (tag == nullptr) return nullptr;
    tag->id = -1;
    m_tags.append(tag);
    return tag;
}

void ApplicationState::freeRecording(Recording *recording)
{
    if (recording == nullptr) return;
    // @TODO: ehh, remove it from m_dirtyRecordings also.
    m_recordingAllocator.deallocate(recording);
}

void ApplicationState::setShouldUpdate(Recording *recording)
{
    // @TODO: This thing doesnt care if recording is already added.
    m_dirtyRecordings.append(recording);
}

void ApplicationState::setShouldUpdate(Session *session)
{
    // @TODO: This thing doesnt care if session is already added.
    m_dirtySessions.append(session);
}

bool ApplicationState::openDatabase()
{
    return m_databaseManager.establishDatabaseConnection();
}

DatabaseManager* ApplicationState::database()
{
    return &m_databaseManager;
}

void ApplicationState::appAboutToQuit()
{
    // @TODO: member access
    Q_ASSERT(m_databaseManager.m_database.isOpen());
    for (int i = 0; i < m_dirtySessions.count(); ++i)
    {
        m_databaseManager.saveSession(m_dirtySessions[i]);
    }
    for (int i = 0; i < m_dirtyRecordings.count(); ++i)
    {
        m_databaseManager.saveRecording(m_dirtyRecordings[i]);
    }
    m_databaseManager.closeDatabaseConnection();
}

ApplicationState g_app;
