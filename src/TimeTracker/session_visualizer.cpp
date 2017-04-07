#include "session_visualizer.h"
#include <QPainter>
#include "application_state.h"
#include <QDebug>

SessionVisualizer::SessionVisualizer(QWidget *parent) : QWidget(parent)
{

}

void SessionVisualizer::setRecordings(QVector<Recording*>* recordings)
{
    //qDebug() << (recordings != nullptr);
    m_recordings = recordings;
    this->update(); // repaint self.
}

void SessionVisualizer::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    static QColor colors[] = { QColor(255,0,0,255), QColor(0,255,0,255), QColor(0,0,255,255) };

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(QColor(255,255,255,255)));
    painter.drawRect(0, 0, width(), height());
    QPoint pos = this->pos();

    if (m_recordings == nullptr || m_recordings->count() == 0)
    {
        return;
    }

    g_app.recorder()->updateRecordingAndIntervalState();

    qint64 min = m_recordings->first()->startTime;
    qint64 max = 0;
    for (int j = 0; j < m_recordings->count(); ++j)
    {
        // @TODO: fix this shitty code
        if (m_recordings->at(j)->endTime > max)
        {
            max = m_recordings->at(j)->endTime;
        }
    }
    qint64 diff = max - min;
    //qDebug() << min << "-" << max << "-" << diff;
    Q_ASSERT(diff >= 0); // @TODO: just clamp.
    double ratio = width() / (double)diff;
    //qDebug() << "redraw ratio" << ratio;

    int i = 0;
    for (int j = 0; j < m_recordings->count(); ++j)
    {
        Recording* recording = m_recordings->at(j);
        painter.setBrush(QBrush(colors[i]));
        for (int k = 0; k < recording->intervals.count(); ++k)
        {
            const Interval2& interval = recording->intervals.at(k);
            double start = (interval.startTime - min) * ratio;
            double end   = (interval.endTime   - min) * ratio;
            painter.drawRect(QRectF(start, 0, end - start, height()));
            //qDebug() << start << "-" << end;
        }
        i = (i + 1) % 3;
    }
    /*
     * - 0 3.61377
     * - 22.53 25.702
     * - 62.90 67.62
     * - 96.21 100
     */

    // min = 1600;
    // max = 3300;
    // diff = 1700;
    // ratio = 100/1700=1/17=0.05
    // i1 st = 1600; // 0
    // i1 et = 2000; // 0.05*400
    // i2 st = 2400;
    // i2 et = 3300;
}

QSize SessionVisualizer::sizeHint() const
{
    return QSize(100, 100);
}
