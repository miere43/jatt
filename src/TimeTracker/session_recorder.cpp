#include "session_recorder.h"

#include <QDebug>
#include "common.h"

SessionRecorder::SessionRecorder(QObject *parent)
    : QObject(parent)
{
}

void SessionRecorder::updateRecordingAndIntervalState()
{
    if (isRecording())
    {
        m_recordEndDateTime   = QDateTime::currentDateTime();

        m_recordingInterval->startTime = m_recordStartDateTime.toMSecsSinceEpoch();
        m_recordingInterval->endTime   = m_recordEndDateTime.toMSecsSinceEpoch();

        m_recording->endTime = m_recordEndDateTime.toMSecsSinceEpoch();
    }
}

bool SessionRecorder::record(Session *session, Recording* recording)
{
    if (session == nullptr)
    {
        APP_ERRSTREAM << "session is null";
        return false;
    }

    if (recording == nullptr)
    {
        APP_ERRSTREAM << "interval is null";
        return false;
    }

    if (isRecording())
    {
        APP_ERRSTREAM << "already recording";
        return false;
    }

    m_session   = session;
    m_recording = recording;

    m_recordStartDateTime = QDateTime::currentDateTime();
    m_recordEndDateTime   = m_recordStartDateTime;

    Interval2 interval;
    interval.startTime = m_recordStartDateTime.toMSecsSinceEpoch();
    interval.endTime = m_recordEndDateTime.toMSecsSinceEpoch();
    if (m_recording->intervals.count() == 0)
    {
        m_recording->startTime = interval.startTime;
    }
    m_recording->endTime = interval.endTime;
    m_recording->intervals.append(interval);
    m_recordingInterval = &m_recording->intervals.last();

    emit recordStateChanged(this, RecordingState::Started);
    return true;
}

bool SessionRecorder::isRecording() const
{
    return m_session != nullptr;
}

qint64 SessionRecorder::msecsPassed()
{
    if (isRecording())
    {
        return m_recordStartDateTime.msecsTo(QDateTime::currentDateTime());
    }
    else
    {
        APP_ERRSTREAM << "invalid state";
        return 0;
    }
}

void SessionRecorder::stop()
{
    if (!isRecording())
    {
        APP_ERRSTREAM << "not recording";
        return;
    }

    // @TODO: actually this stuff should update every timer tick, because all of this stuff is
    // used in rendering, also this stuff could be saved to database in any time.

    m_recordEndDateTime = QDateTime::currentDateTime();
    // @TODO: Two calls to fromMSecsSinceEpoch may introduce minor precision error?
    m_recordingInterval->startTime = QDateTime::fromMSecsSinceEpoch(0).msecsTo(m_recordStartDateTime.toUTC());
    m_recordingInterval->endTime   = QDateTime::fromMSecsSinceEpoch(0).msecsTo(m_recordEndDateTime.toUTC());
    qDebug().nospace() << "Recording stopped for " << m_recording->name << " (start: " << m_recordingInterval->startTime << " ms, end: " << m_recordingInterval->endTime << " ms, diff: " << m_recordingInterval->endTime - m_recordingInterval->startTime << ")";

    Q_ASSERT(m_recordingInterval->startTime >= 0);
    Q_ASSERT(m_recordingInterval->endTime   >= 0);

    if (m_recording->intervals.count() == 1) {
        m_recording->startTime = m_recordingInterval->startTime;
    }
    m_recording->endTime = m_recordingInterval->endTime;

    emit recordStateChanged(this, RecordingState::Finished);

    m_session  = nullptr;
    m_recording = nullptr;
}

Recording* SessionRecorder::recording() const
{
    return m_recording;
}

Interval2* SessionRecorder::recordingInterval() const
{
    return m_recordingInterval;
}

Session* SessionRecorder::session() const
{
    return m_session;
}
