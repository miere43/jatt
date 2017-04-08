#ifndef SESSION_VISUALIZER_H
#define SESSION_VISUALIZER_H

#include <QWidget>
#include <QPaintEvent>
#include "core_types.h"

class ActivityVisualizer : public QWidget
{
    Q_OBJECT
public:
    enum TimelineRenderMode
    {
        Full,
        Effective,
    };

    explicit ActivityVisualizer(QWidget *parent = 0);

    void setTimePeriod(qint64 startTime, qint64 endTime, QVector<Activity*>* activities);
    void setTimelineRenderMode(TimelineRenderMode mode);

    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const;
private:
    qint64 m_startTime;
    qint64 m_endTime;
    TimelineRenderMode m_timelineRenderMode = Effective;
    QVector<Activity*>* m_activities;
};

#endif // SESSION_VISUALIZER_H
