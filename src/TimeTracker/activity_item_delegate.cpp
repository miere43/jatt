#include "activity_item_delegate.h"
#include "activity_list_model.h"

#include "core_types.h"
#include "error_macros.h"

ActivityItemDelegate::ActivityItemDelegate()
{

}

void ActivityItemDelegate::setCurrentActivity(Activity * activity)
{
    m_currentActivity = activity;
}

Activity* ActivityItemDelegate::currentActivity() const
{
    return m_currentActivity;
}

void ActivityItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    static const QBrush g_currentActivityBrush = QBrush((QColor(224, 248, 255)));

    const Activity * activity = (Activity *)index.data(Qt::UserRole).value<void *>();
    ERR_VERIFY_NULL(activity);

    painter->setBrush(m_currentActivity == activity ? g_currentActivityBrush : option.backgroundBrush);
    painter->setPen(Qt::NoPen);
    painter->drawRect(option.rect);

    QString displayText = index.data(Qt::DisplayRole).value<QString>();

    const int fontHeight = option.fontMetrics.height();

    QPen textPen;
    QRect textPos = option.rect;
    textPos.adjust(fontHeight, 0, 0, 0);

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
    painter->setPen(QPen(QColor(0, 0, 0), 0.8));
    painter->setBrush(QBrush(QColor((QRgb)activity->category->color)));
    painter->drawEllipse(QRect(option.rect.x(), option.rect.y(), fontHeight, fontHeight).marginsRemoved(QMargins(4, 4, 4, 4)));
}

QSize ActivityItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    const int ellipseWidth = option.fontMetrics.height() + (4 * 2);
    const int textWidth    = option.fontMetrics.width(index.data(Qt::DisplayRole).value<QString>());
    return QSize(ellipseWidth + textWidth, option.fontMetrics.height());
}
