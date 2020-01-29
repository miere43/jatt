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
    static const QBrush g_favoriteActivityBrush = QBrush((QColor(255, 216, 0)));

    auto activity = reinterpret_cast<const Activity *>(index.data(Qt::UserRole).value<void *>());
    ERR_VERIFY_NULL(activity);

    painter->setBrush(m_currentActivity == activity ? g_currentActivityBrush : option.backgroundBrush);
    painter->setPen(Qt::NoPen);
    painter->drawRect(option.rect);

    QString displayText = index.data(Qt::DisplayRole).value<QString>();

    const int fontHeight = option.fontMetrics.height();

    QPen textPen;
    QRect textPos = option.rect;
    textPos.adjust(fontHeight, 0, 0, 0);

    if (activity->favorite)
    {
        textPos.setX(textPos.x() + 10);
    }

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
    painter->setBrush(QBrush(QColor(static_cast<QRgb>(activity->category->color))));
    painter->drawEllipse(QRect(option.rect.x(), option.rect.y(), fontHeight, fontHeight).marginsRemoved(QMargins(4, 4, 4, 4)));

    if (activity->favorite)
    {
        painter->setBrush(g_favoriteActivityBrush);

        double ox = option.rect.x() + 10;
        double oy = option.rect.y() - 1;
        painter->save();
        painter->translate(ox, oy);
        QPointF points[] = {
            { 8, 4 },
            { 9, 6.5 },
            { 12, 6.5 },
            { 10, 9 },
            { 11, 12 },
            { 8, 10.5 },
            { 5, 12 },
            { 6, 9 },
            { 4, 6.5 },
            { 7, 6.5 },
        };
        painter->setPen(QPen(QColor(0, 0, 0), 0.7));
        painter->drawPolygon(points, sizeof(points) / sizeof(points[0]), Qt::WindingFill);
        painter->restore();
    }
}

QSize ActivityItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    auto text = index.data(Qt::DisplayRole).value<QString>();
    auto rect = option.fontMetrics.boundingRect(text);
    const int ellipseWidth = option.fontMetrics.height() + (4 * 2);
    const int textWidth    = rect.width();
    return QSize(ellipseWidth + textWidth, option.fontMetrics.height());
}
