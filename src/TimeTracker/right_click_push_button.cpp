#include "right_click_push_button.h"

RightClickPushButton::RightClickPushButton(QWidget* parent)
    : QPushButton(parent)
{
}

void RightClickPushButton::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::RightButton)
    {
        emit rightClicked();
        e->accept();
    }
    else
    {
        QPushButton::mousePressEvent(e);
    }
}
