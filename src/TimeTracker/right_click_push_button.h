#ifndef RIGHT_CLICK_PUSH_BUTTON_H
#define RIGHT_CLICK_PUSH_BUTTON_H

#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>

class RightClickPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit RightClickPushButton(QWidget* parent = nullptr);
signals:
    void rightClicked();
private slots:
    virtual void mousePressEvent(QMouseEvent* e) override;
};

#endif // RIGHT_CLICK_PUSH_BUTTON_H
