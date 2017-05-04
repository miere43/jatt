#ifndef ACTIVITY_ITEM_DELEGATE_H
#define ACTIVITY_ITEM_DELEGATE_H

#include <QAbstractItemDelegate>
#include <QPainter>
#include <QBrush>

struct Activity;

class ActivityItemDelegate : public QAbstractItemDelegate
{
public:
    ActivityItemDelegate();

    void setCurrentActivity(Activity* activity);
    Activity* currentActivity() const;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
    QBrush m_currentActivityBrush = QBrush((QColor(224, 248, 255)));
    Activity* m_currentActivity = nullptr;
};

#endif // ACTIVITY_ITEM_DELEGATE_H
