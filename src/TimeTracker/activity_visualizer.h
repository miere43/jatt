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
//    void selectInterval(const Interval* interval);
    void selectActivity(const Activity* activity);

    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const;
private slots:
    void switchRenderModeTriggered(bool checked);
    void contextMenuRequested(const QPoint& pos);
private:
    qint64 m_startTime;
    qint64 m_endTime;
    TimelineRenderMode m_timelineRenderMode = Effective;
    const Activity* m_selectedActivity = nullptr;
    QVector<Activity*>* m_activities;

    QAction* m_mode;
    QMenu* m_menu;
};

#endif // SESSION_VISUALIZER_H
