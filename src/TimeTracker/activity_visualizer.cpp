#include "activity_visualizer.h"
#include <QPainter>
#include "application_state.h"
#include <QDebug>

ActivityVisualizer::ActivityVisualizer(QWidget *parent) : QWidget(parent)
{
    connect(this, &ActivityVisualizer::customContextMenuRequested,
            this, &ActivityVisualizer::contextMenuRequested);
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    m_menu = new QMenu(this);
    m_mode = m_menu->addAction("Switch render mode");
    connect(m_mode, &QAction::triggered,
            this, &ActivityVisualizer::switchRenderModeTriggered);
}

void ActivityVisualizer::switchRenderModeTriggered(bool checked)
{
    Q_UNUSED(checked);
    TimelineRenderMode m = m_timelineRenderMode == TimelineRenderMode::Effective ?
                           TimelineRenderMode::Full : TimelineRenderMode::Effective;
    setTimelineRenderMode(m);
}

void ActivityVisualizer::contextMenuRequested(const QPoint &pos)
{
    m_menu->exec(this->mapToGlobal(pos));
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
    // static QColor colors[] = { QColor(255, 0, 0, 255), QColor(0, 255, 0, 255), QColor(0, 0, 255, 255) };

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
        Q_ASSERT(minTime <= maxTime);
        width = maxTime - minTime;
        if (width == 0)
            return; // Avoid division by zero.
        unit = (double)this->width() / width;
        startTime = minTime;
    }
    else
    {
        Q_ASSERT(false);
    }

    QBrush currentBrush;
    int i = 0;
    Q_UNUSED(i);
    for (const Activity* activity : *m_activities)
    {
        Q_ASSERT(activity);
        if (activity->intervals.count() == 0)
            continue;
        currentBrush = activity == m_selectedActivity ? QBrush(QColor(127, 201, 255, 255)) : QBrush(QColor((QRgb)activity->info->color));
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
}

QSize ActivityVisualizer::sizeHint() const
{
    return QSize(100, 100);
}

void ActivityVisualizer::selectActivity(const Activity *activity)
{
    if (activity == nullptr) {
        m_selectedActivity = nullptr;
    } else {
        m_selectedActivity = activity;
    }
    this->update();
}
