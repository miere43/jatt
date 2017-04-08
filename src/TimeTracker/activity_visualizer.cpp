#include "activity_visualizer.h"
#include <QPainter>
#include "application_state.h"
#include <QDebug>

ActivityVisualizer::ActivityVisualizer(QWidget *parent) : QWidget(parent)
{

}

void ActivityVisualizer::setTimelineRenderMode(TimelineRenderMode mode)
{
    if (m_timelineRenderMode == mode)
        return;
    m_timelineRenderMode = mode;
    this->update();
}

void ActivityVisualizer::setTimePeriod(qint64 startTime, qint64 endTime, QVector<Activity *> *activities)
{
    Q_ASSERT(activities);
    m_startTime = startTime;
    m_endTime = endTime;
    m_activities = activities;
    this->update();
}

void ActivityVisualizer::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    static QColor colors[] = { QColor(255, 0, 0, 255), QColor(0, 255, 0, 255), QColor(0, 0, 255, 255) };

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(QColor(255,255,255,255)));
    painter.drawRect(0, 0, width(), height());

    if (m_activities == nullptr || m_activities->isEmpty())
    {
        return;
    }

    qint64 startTime;
    qint64 width = 1;
    double unit = 1;
    if (m_timelineRenderMode == Full)
    {
        width = m_endTime - m_startTime;
        unit  = (double)this->width() / width;
        startTime = m_startTime;
    }
    else if (m_timelineRenderMode == Effective)
    {
        qint64 minTime = INT64_MAX;
        qint64 maxTime = INT64_MIN;
        for (const Activity* activity : *m_activities)
        {
            if (activity->startTime < minTime)
            {
                minTime = activity->startTime;
            }
            if (activity->endTime > maxTime)
            {
                maxTime = activity->endTime;
            }
        }
        Q_ASSERT(minTime <= maxTime);
        width = maxTime - minTime;
        qDebug() << "width:" << width;
        unit = (double)this->width() / width;
        qDebug() << "unit:" << unit;
        startTime = minTime;
        qDebug() << "startTime:" << startTime;
    }
    else
    {
        Q_ASSERT(false);
    }

    int i = 0;
    for (const Activity* activity : *m_activities)
    {
        Q_ASSERT(activity);
        if (activity->intervals.count() > 0)
            painter.setBrush(QBrush(colors[(i++) % 3]));
        qDebug() << "render" << i-1;

        for (const Interval& interval : activity->intervals)
        {
            double startPixel = 1;
            double endPixel = 1;

                startPixel = (interval.startTime - startTime) * unit;
                endPixel   = (interval.endTime     - startTime) * unit;

            if (endPixel - startPixel < 1) endPixel += 1;
                    qDebug() << "startPixel" << startPixel << "endPixel" << endPixel;
            painter.drawRect(QRectF(startPixel, 0, endPixel - startPixel, height()));
        }
    }
}

QSize ActivityVisualizer::sizeHint() const
{
    return QSize(100, 100);
}
