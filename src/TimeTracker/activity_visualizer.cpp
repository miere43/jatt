#include "activity_visualizer.h"
#include "application_state.h"
#include "error_macros.h"
#include "utilities.h"

#include <QtGlobal>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QFontMetricsF>

ActivityVisualizer::ActivityVisualizer(QWidget * parent) : QWidget(parent)
{
    this->setMinimumHeight(50);
    this->setCursor(Qt::PointingHandCursor);

    m_dropShadow = new QGraphicsDropShadowEffect(this);
    m_dropShadow->setColor(QColor(0, 0, 0, 255));
    m_dropShadow->setBlurRadius(5.0);
    m_dropShadow->setOffset(5.0);

//    this->setGraphicsEffect(m_dropShadow);
}

ActivityVisualizer::~ActivityVisualizer()
{
    delete m_dropShadow;
}

bool ActivityVisualizer::isPointInSelection(const QPoint & point)
{
    if (!m_hasSelection) return false;

    int x = point.x();
    if (m_selectionAreaStart < m_selectionAreaEnd)
        return x >= m_selectionAreaStart && x <= m_selectionAreaEnd;
    else
        return x >= m_selectionAreaEnd && x <= m_selectionAreaStart;
}

void ActivityVisualizer::setTimePeriod(qint64 startTime, qint64 endTime, QVector<Activity *> * activities)
{
    ERR_VERIFY_NULL(activities);

    m_startTime = startTime;
    m_endTime = endTime;
    m_activities = activities;
    clearSelection();

    this->update();
}

void ActivityVisualizer::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(QColor(255, 255, 255, 255)));
    painter.drawRect(0, 0, width(), height());

    if (m_activities != nullptr && !m_activities->isEmpty())
    {
        qint64 width = m_endTime - m_startTime;
        if (width == 0) width = 1;
        const double unit  = static_cast<double>(this->width()) / width;

        QBrush currentBrush;
        for (const Activity * activity : *m_activities)
        {
            if (activity->intervals.count() == 0)
                continue;
            currentBrush = activity == m_activeActivity ? QBrush(QColor(127, 201, 255, 255)) : QBrush(QColor(static_cast<QRgb>(activity->category->color)));
            painter.setBrush(currentBrush);

            for (const Interval& interval : activity->intervals)
            {
                double startPixel = (interval.startTime - m_startTime) * unit;
                double endPixel   = (interval.endTime   - m_startTime) * unit;

                if (endPixel - startPixel < 1) endPixel += 1;
                if (qIsInf(startPixel) || qIsInf(endPixel)) continue;
                painter.drawRect(QRectF(startPixel, 0, endPixel - startPixel, height()));
            }
        }
    }

    if (m_hasSelection && m_selectionAreaStart != m_selectionAreaEnd)
    {
        static QBrush selectionBrush = QBrush(QColor(0, 0, 255, 100));
        painter.setBrush(selectionBrush);

        QRectF selectionRect = QRectF(m_selectionAreaStart, 0, m_selectionAreaEnd - m_selectionAreaStart, height());
        const int textFlags = Qt::TextDontClip | Qt::AlignHCenter | Qt::AlignVCenter;

        painter.drawRect(selectionRect);

        painter.setFont(this->font());

        QFontMetricsF metrics(this->font());
        QRectF dropShadowRect = metrics
                .boundingRect(selectionRect, Qt::AlignHCenter | Qt::AlignVCenter, m_selectionTimeText)
                .marginsAdded(QMarginsF(2.0, 2.0, 2.0, 2.0));

        painter.setBrush(QBrush(QColor(0, 0, 0, 100)));
        painter.drawRect(dropShadowRect);

        painter.setPen(QPen(QColor(255, 255, 255, 255)));
        painter.drawText(selectionRect, textFlags, m_selectionTimeText);

    }
}

void ActivityVisualizer::clearSelection()
{
    m_selecting = false;
    m_hasSelection = false;
    m_selectionAreaStart = 0;
    m_selectionAreaEnd = 0;

    this->update(); // @TODO: update only selection region.
}

void ActivityVisualizer::mousePressEvent(QMouseEvent * event)
{
    if (!(event->button() & Qt::LeftButton)) return;

    m_selecting = true;
    m_hasSelection = true;
    m_selectionAreaStart = qBound(0, event->x(), this->width());
    m_selectionAreaEnd   = m_selectionAreaStart;

    this->update();
}

void ActivityVisualizer::mouseMoveEvent(QMouseEvent * event)
{
    if (!m_selecting) return;

    m_selectionAreaEnd = qBound(0, event->x(), this->width());

    qint64 startTime, endTime;
    ERR_VERIFY(selectionInterval(&startTime, &endTime));

    m_selectionTimeText = formatDuration(endTime - startTime, false);

    this->update();
}

QSize ActivityVisualizer::sizeHint() const
{
    return QSize(100, 100);
}

void ActivityVisualizer::setActiveActivity(const Activity * activity)
{
    m_activeActivity = activity;
    this->update();
}

bool ActivityVisualizer::selectionInterval(qint64 * startTime, qint64 * endTime)
{
    ERR_VERIFY_NULL_V(startTime, false);
    ERR_VERIFY_NULL_V(endTime, false);

    qint64 msPerPixel = (m_endTime - m_startTime) / static_cast<qint64>(this->width());

    *startTime = m_startTime + (msPerPixel * m_selectionAreaStart);
    *endTime   = m_startTime + (msPerPixel * m_selectionAreaEnd);

    if (*startTime > *endTime)
    {
        qint64 temp = *startTime;
        *startTime  = *endTime;
        *endTime    = temp;
    }

    return true;
}
