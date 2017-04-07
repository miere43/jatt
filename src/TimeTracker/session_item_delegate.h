#ifndef SESSION_ITEM_DELEGATE_H
#define SESSION_ITEM_DELEGATE_H

#include <QStyledItemDelegate>

class SessionItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    SessionItemDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // SESSION_ITEM_DELEGATE_H
