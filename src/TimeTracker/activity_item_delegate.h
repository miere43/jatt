#ifndef ACTIVITY_ITEM_DELEGATE_H
#define ACTIVITY_ITEM_DELEGATE_H

#include <QAbstractItemDelegate>
#include <QPainter>


class ActivityItemDelegate : public QAbstractItemDelegate
{
public:
    ActivityItemDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // ACTIVITY_ITEM_DELEGATE_H
