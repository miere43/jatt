#include "activity_recorder.h"

#include <QDebug>

ActivityRecorder::ActivityRecorder(QObject* parent)
    : QObject(parent)
{
    m_timer.setInterval(200);
    m_timer.setSingleShot(false);
    m_timer.setTimerType(Qt::PreciseTimer);

    connect(&m_timer, &QTimer::timeout,
            this, &ActivityRecorder::timerTimeout);
}

void ActivityRecorder::timerTimeout()
{
    Q_ASSERT(isRecording());

    syncActivityState();
    emit recordEvent(ActivityRecorderEvent::SyncTimer);
}

void ActivityRecorder::setTimerType(Qt::TimerType type)
{
    Q_ASSERT(!isRecording());
    m_timer.setTimerType(type);
}

void ActivityRecorder::setTimerInterval(int msecs)
{
    Q_ASSERT(!isRecording());
    m_timer.setInterval(msecs);
}

void ActivityRecorder::record(Activity *activity)
{
    Q_ASSERT(activity);
    Q_ASSERT(!isRecording());

    m_isRecording = true;
    m_startTime = getCurrentDateTimeUtc();
    m_activity = activity;

    if (activity->intervals.count() == 0)
    {
        activity->startTime = m_startTime;
        activity->endTime = m_startTime;
    }

    // @TODO @FIXME @CRITICAL:
    // very dangerous stuff going on here, this memory can be
    // relocated to another location when new item is added to
    // activity->intervals
    // when this will happen, pointer to m_activityInterval will become invalid.
    // I guess storing an index will be more safe?
    activity->intervals.append(Interval { m_startTime, m_startTime });
    m_activityInterval = &activity->intervals.last();

//    g_app.database().saveActivity(activity);
    emit recordEvent(ActivityRecorderEvent::RecordingStarted);

    m_timer.start();
}

void ActivityRecorder::stop()
{
    Q_ASSERT(isRecording());

    m_timer.stop();
    syncActivityState();
    m_isRecording = false;

    emit recordEvent(ActivityRecorderEvent::RecordingStopped);
}

void ActivityRecorder::syncActivityState()
{
    Q_ASSERT(isRecording());
    m_activity->endTime = getCurrentDateTimeUtc();
    m_activityInterval->endTime = m_activity->endTime;
}

inline qint64 qint64_max(qint64 a, qint64 b) {
    return a > b ? a : b;
}

qint64 ActivityRecorder::duration() const {
    if (!isRecording()) return 0;
    return qint64_max(0, getCurrentDateTimeUtc() - m_activityInterval->startTime);
}
