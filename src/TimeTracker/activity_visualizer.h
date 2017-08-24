#ifndef SESSION_VISUALIZER_H
#define SESSION_VISUALIZER_H

#include <QWidget>
#include <QMenu>
#include <QAction>
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

    void setActiveActivity(const Activity* activity);

    // 'point' should in window coordinate space.
    bool isPointInSelection(const QPoint & point);
    void clearSelection();

    inline int selectionStart() const { return m_selectionAreaStart; }
    inline int selectionLength() const { return m_selectionAreaLength; }

    void selectionInterval(qint64 & startTime, qint64 & endTime);

    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;

    QSize sizeHint() const;
private:
    qint64 m_startTime;
    qint64 m_endTime;
    TimelineRenderMode m_timelineRenderMode = Full;
    const Activity * m_activeActivity = nullptr;
    QVector<Activity *> * m_activities;

    bool m_selecting    = false;
    bool m_hasSelection = false;
    int m_selectionAreaStart = 0;  // in pixels.
    int m_selectionAreaLength = 0;
};

#endif // SESSION_VISUALIZER_H
