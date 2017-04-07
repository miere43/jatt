#ifndef SESSION_VISUALIZER_H
#define SESSION_VISUALIZER_H

#include <QWidget>
#include <QPaintEvent>
#include "core_types.h"

class SessionVisualizer : public QWidget
{
    Q_OBJECT
public:
    explicit SessionVisualizer(QWidget *parent = 0);

    void setRecordings(QVector<Recording*>* recordings);

    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const;
signals:

public slots:

private:
    /**
     * @brief Represents array of Recordings that will be rendered in this widget.
     */
    QVector<Recording*>* m_recordings = nullptr;
};

#endif // SESSION_VISUALIZER_H
