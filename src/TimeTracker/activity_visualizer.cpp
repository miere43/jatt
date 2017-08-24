#include "activity_visualizer.h"
#include "application_state.h"
#include "error_macros.h"

#include <QPainter>
#include <QDebug>

ActivityVisualizer::ActivityVisualizer(QWidget * parent) : QWidget(parent)
{
    this->setMinimumHeight(50);
}

bool ActivityVisualizer::isPointInSelection(const QPoint &point)
{
    if (!m_hasSelection || m_selectionAreaLength == 0) return false;

    int x = point.x();
    if (m_selectionAreaLength > 0)
        return x >= m_selectionAreaStart && x <= (m_selectionAreaStart + m_selectionAreaLength);
    else
        return x <= m_selectionAreaStart && x >= (m_selectionAreaStart + m_selectionAreaLength);
}

void ActivityVisualizer::setTimelineRenderMode(TimelineRenderMode mode)
{
    if (m_timelineRenderMode == mode)
        return;
    m_timelineRenderMode = mode;
    this->update();
}

void ActivityVisualizer::setTimePeriod(qint64 startTime, qint64 endTime, QVector<Activity *> * activities)
{
    ERR_VERIFY_NULL(activities);

    m_startTime = startTime;
    m_endTime = endTime;
    m_activities = activities;

    this->update();
}

void ActivityVisualizer::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event);

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
        if (width == 0) return;
        unit  = (double)this->width() / width;
        startTime = m_startTime;
    }
    else if (m_timelineRenderMode == Effective)
    {
        qint64 minTime = INT64_MAX;
        qint64 maxTime = INT64_MIN;
        for (const Activity * activity : *m_activities)
        {
            if (activity->intervals.count() == 0)
                continue;
            if (activity->startTime < minTime)
            {
                minTime = activity->startTime;
            }
            if (activity->endTime > maxTime)
            {
                maxTime = activity->endTime;
            }
        }
        if (minTime == INT64_MAX && maxTime == INT64_MIN)
            return; // Nothing to render.
        ERR_VERIFY(minTime <= maxTime);
        width = maxTime - minTime;
        if (width == 0) return; // Avoid division by zero.
        unit = (double)this->width() / width;
        startTime = minTime;
    }
    else
    {
        ERR_VERIFY(false);
    }

    QBrush currentBrush;
    for (const Activity * activity : *m_activities)
    {
        if (activity->intervals.count() == 0)
            continue;
        currentBrush = activity == m_activeActivity ? QBrush(QColor(127, 201, 255, 255)) : QBrush(QColor((QRgb)activity->category->color));
        painter.setBrush(currentBrush);

        for (const Interval& interval : activity->intervals)
        {
            double startPixel = (interval.startTime - startTime) * unit;
            double endPixel   = (interval.endTime   - startTime) * unit;

            if (endPixel - startPixel < 1) endPixel += 1;
            if (startPixel == INFINITY || endPixel == INFINITY) continue;
            painter.drawRect(QRectF(startPixel, 0, endPixel - startPixel, height()));
        }
    }

    if (m_hasSelection && m_selectionAreaLength != 0)
    {
        static QBrush selectionBrush = QBrush(QColor(0, 0, 255, 100));
        painter.setBrush(selectionBrush);

        painter.drawRect(QRectF(m_selectionAreaStart, 0, m_selectionAreaLength, height()));
    }
}

void ActivityVisualizer::clearSelection()
{
    m_selecting = false;
    m_hasSelection = false;
    m_selectionAreaStart = 0;
    m_selectionAreaLength = 0;

    this->update(); // @TODO: update only selection region.
}

void ActivityVisualizer::mousePressEvent(QMouseEvent * event)
{
    if (!(event->button() & Qt::LeftButton)) return;

    m_selecting = true;
    m_hasSelection = true;
    m_selectionAreaStart = event->x();
    m_selectionAreaLength = 0;
}

void ActivityVisualizer::mouseMoveEvent(QMouseEvent * event)
{
    if (!m_selecting) return;
    // @TODO: mouse leave
    m_selectionAreaLength = event->x() - m_selectionAreaStart;

    // @TODO: only update region of selection area
    // if area increased: update from start to start+length
    // if area decreased: update old area from start to start+length
    this->update();
}

QSize ActivityVisualizer::sizeHint() const
{
    return QSize(100, 100);
}

void ActivityVisualizer::setActiveActivity(const Activity * activity)
{
    if (activity == nullptr)
        m_activeActivity = nullptr;
    else
        m_activeActivity = activity;
    this->update();
}

void ActivityVisualizer::selectionInterval(qint64 & startTime, qint64 & endTime)
{
    int start, length;
    if (m_selectionAreaLength < 0)
    {
        length = -m_selectionAreaLength;
        start = m_selectionAreaStart - length;
    }
    else
    {
        length = m_selectionAreaLength;
        start = m_selectionAreaStart;
    }

    ERR_VERIFY(start > 0);
    ERR_VERIFY(length > 0);

    qint64 msPerPixel = (m_endTime - m_startTime) / (qint64)this->width();
    ERR_VERIFY(msPerPixel > 0);

    startTime = m_startTime + (msPerPixel * start);
    endTime   = startTime + (msPerPixel * length);
}
