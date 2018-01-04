#ifndef SESSION_VISUALIZER_H
#define SESSION_VISUALIZER_H

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QPaintEvent>
#include <QGraphicsDropShadowEffect>
#include "core_types.h"

class ActivityVisualizer : public QWidget
{
    Q_OBJECT
public:
    explicit ActivityVisualizer(QWidget * parent = 0);
    ~ActivityVisualizer();

    void setTimePeriod(qint64 startTime, qint64 endTime, QVector<Activity *> * activities);

    void setActiveActivity(const Activity * activity);

    // 'point' should in window coordinate space.
    bool isPointInSelection(const QPoint & point);
    void clearSelection();

    inline int selectionStart() const { return m_selectionAreaStart; }
    inline int selectionEnd() const { return m_selectionAreaEnd; }

    bool selectionInterval(qint64 * startTime, qint64 * endTime);

    void paintEvent(QPaintEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;

    QSize sizeHint() const;
private:
    qint64 m_startTime;
    qint64 m_endTime;
    const Activity * m_activeActivity = nullptr;
    QVector<Activity *> * m_activities;

    bool m_selecting    = false;
    bool m_hasSelection = false;
    int m_selectionAreaStart = 0;  // in pixels.
    int m_selectionAreaEnd   = 0;
    QString m_selectionTimeText;

    QGraphicsDropShadowEffect* m_dropShadow;
};

#endif // SESSION_VISUALIZER_H
