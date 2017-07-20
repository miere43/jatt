#ifndef ACTIVITY_RECORDER_H
#define ACTIVITY_RECORDER_H

#include <core_types.h>
#include <QTimer>

enum class ActivityRecorderEvent
{
    RecordingStarted,
    UpdateUITimer,
    RecordingStopped,
    Autosave,
};

class ActivityRecorder : public QObject
{
    Q_OBJECT

public:
    ActivityRecorder(QObject* parent = nullptr);

    void setTimerType(Qt::TimerType type);
    void setTimerInterval(int msecs);

    void setAutosaveTimerInterval(int msecs);

    void record(Activity* activity);
    void stop();

    void syncActivityState();

    inline bool isRecording() const { return m_isRecording; }
    inline Activity* activity() const { return m_activity; }
    inline Interval* interval() const { return m_activityInterval; }
    qint64 duration() const;
signals:
    void recordEvent(ActivityRecorderEvent state);
private slots:
    void timerTimeout();
    void autosaveTimerTimeout();
private:
    qint64 m_startTime;
    Interval* m_activityInterval = nullptr;
    Activity* m_activity = nullptr;
    bool m_isRecording = false;
    QTimer m_timer;
    QTimer m_autosaveTimer;
};

#endif // ACTIVITY_RECORDER_H
