#include "session_item_delegate.h"

#include <QStyleOptionViewItem>
#include <QPainter>
#include <QDebug>
#include "core_types.h"

SessionItemDelegate::SessionItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void SessionItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, option.palette.highlight());
    }

    QVariant sessionVariant = index.model()->data(index, Qt::UserRole);
    Session* session = nullptr;
    if (!sessionVariant.isValid() || (session = (Session*)sessionVariant.value<void*>()) == nullptr)
    {
        qWarning() << __FUNCTION__ << "- invalid session received for UserRole";
        sessionVariant = index.model()->data(index, Qt::DisplayRole);
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setFont(option.font);

    if (option.state & QStyle::State_Selected)
    {
        painter->setPen(QPen(option.palette.highlightedText(), 1));
    }
    else
    {
        painter->setPen(QPen(option.palette.text(), 1));
    }

    painter->drawText(option.rect, session->name);

    if (!session->note.isEmpty())
    {
        //QSize size = sizeHint(option, index);
        QRect renderRect { option.rect.x() + option.rect.width() - option.rect.height() + 4,
                           option.rect.y() + 4,
                           option.rect.height() - 8,
                           option.rect.height() - 8};
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor { 255, 216, 0, 255 });
        painter->drawEllipse(renderRect);
    }

    painter->restore();
}

QSize SessionItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // @TODO: option.rect.width() reports weird size, when resizing main window it stays the same
    // and creates scroll bar when not needed.
    Q_UNUSED(index);
    return QSize(option.rect.width(), option.fontMetrics.height());
}
