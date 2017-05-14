#include "activity_item_delegate.h"
#include "activity_list_model.h"

#include "core_types.h"
#include "error_macros.h"

ActivityItemDelegate::ActivityItemDelegate()
{

}

void ActivityItemDelegate::setCurrentActivity(Activity *activity)
{
    m_currentActivity = activity;
}

Activity* ActivityItemDelegate::currentActivity() const
{
    return m_currentActivity;
}

void ActivityItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Activity* activity = (Activity*)index.data(Qt::UserRole).value<void*>();
    ERR_VERIFY_NULL(activity);

    if (m_currentActivity == activity) {
        painter->setBrush(m_currentActivityBrush);
    } else {
        painter->setBrush(option.backgroundBrush);
    }
    painter->setPen(Qt::NoPen);
    painter->drawRect(option.rect);

    QString displayText = index.data(Qt::DisplayRole).value<QString>();

    QPen textPen;
    QRect textPos = option.rect;
    textPos.adjust(option.fontMetrics.height(), 0, 0, 0);

    if (option.state & QStyle::State_Selected)
    {
        painter->setBrush(option.palette.highlight());
        painter->drawRect(option.rect);

        textPen = QPen(option.palette.highlightedText(), 1);
    }
    else
    {
        textPen = QPen(option.palette.text(), 1);
    }

    painter->setPen(textPen);
    painter->drawText(textPos, displayText);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(QColor(0,0,0), 0.8));
    painter->setBrush(QBrush(QColor((QRgb)activity->info->color)));
    painter->drawEllipse(QRect(option.rect.x(), option.rect.y(), option.fontMetrics.height(), option.fontMetrics.height()).marginsRemoved(QMargins(4,4,4,4)));
}

QSize ActivityItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QSize(option.rect.width(), option.fontMetrics.height());
}
