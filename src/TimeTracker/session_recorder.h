#ifndef SESSION_RECORDER_H
#define SESSION_RECORDER_H

#include <QDateTime>
#include <QTime>
#include <QTimer>
#include <QtGlobal>
#include <QObject>
#include "core_types.h"

enum class RecordingState
{
    Started,
    Finished
};

class SessionRecorder : public QObject
{
    Q_OBJECT

public:
    explicit SessionRecorder(QObject *parent = nullptr);

    Session*   m_session = nullptr;
    Recording* m_recording = nullptr;
    Interval2*  m_recordingInterval = nullptr;

    bool record(Session* session, Recording* recording);
    void stop();
    qint64 msecsPassed();
    bool isRecording() const;

    void updateRecordingAndIntervalState();

    Recording* recording() const;
    Interval2*  recordingInterval() const;
    Session*   session() const;
private:
    QDateTime m_recordStartDateTime;
    QDateTime m_recordEndDateTime;

signals:
    void recordStateChanged(SessionRecorder* recorder, RecordingState state);

private slots:
};

#endif // SESSION_RECORDER_H
