#include "activity_recorder.h"
#include "error_macros.h"

#include <QtGlobal>
#include <QDebug>

ActivityRecorder::ActivityRecorder(QObject* parent)
    : QObject(parent)
{
    m_timer.setInterval(500);
    m_timer.setSingleShot(false);
    m_timer.setTimerType(Qt::PreciseTimer);

    m_autosaveTimer.setInterval(60 * 1000);
    m_autosaveTimer.setSingleShot(false);
    m_autosaveTimer.setTimerType(Qt::CoarseTimer);

    connect(&m_timer, &QTimer::timeout,
            this, &ActivityRecorder::timerTimeout);
    connect(&m_autosaveTimer, &QTimer::timeout,
            this, &ActivityRecorder::autosaveTimerTimeout);
}

void ActivityRecorder::timerTimeout()
{
    ERR_VERIFY(isRecording());

    syncActivityState();
    emit recordEvent(ActivityRecorderEvent::UpdateUITimer);
}

void ActivityRecorder::autosaveTimerTimeout()
{
    ERR_VERIFY(isRecording());

    syncActivityState();
    emit recordEvent(ActivityRecorderEvent::Autosave);
}

void ActivityRecorder::setTimerType(Qt::TimerType type)
{
    ERR_VERIFY(!isRecording());

    m_timer.setTimerType(type);
}

void ActivityRecorder::setTimerInterval(int msecs)
{
    ERR_VERIFY(!isRecording());

    m_timer.setInterval(msecs);
}

void ActivityRecorder::record(Activity * activity)
{
    ERR_VERIFY_NULL(activity);
    ERR_VERIFY(!isRecording());

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
    m_autosaveTimer.start();
}

void ActivityRecorder::stop()
{
    ERR_VERIFY(isRecording());

    m_timer.stop();
    m_autosaveTimer.stop();
    syncActivityState();
    m_isRecording = false;

    emit recordEvent(ActivityRecorderEvent::RecordingStopped);
}

void ActivityRecorder::syncActivityState()
{
    ERR_VERIFY(isRecording());

    m_activity->endTime = getCurrentDateTimeUtc();
    m_activityInterval->endTime = m_activity->endTime;
}

qint64 ActivityRecorder::duration() const
{
    if (!isRecording()) return 0;
    return qMax<qint64>(0, getCurrentDateTimeUtc() - m_activityInterval->startTime);
}

void ActivityRecorder::setAutosaveTimerInterval(int msecs)
{
    m_autosaveTimer.setInterval(msecs);
}
