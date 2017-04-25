#ifndef ACTIVITYLISTMODEL_H
#define ACTIVITYLISTMODEL_H

#include <QAbstractListModel>

#include "core_types.h"

struct ActivityListItem {
    const Interval* interval;
    Activity* activity;
};

bool operator<(const ActivityListItem& lhs, const ActivityListItem& rhs);

class ActivityListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ActivityListModel(QObject *parent = 0);

    void addActivity(Activity* activity);
    void addActivities(QVector<Activity*>* activities);
    void addActivityInterval(Activity* activity, const Interval* interval, bool forceSort = true);

    void setTimePeriod(qint64 startTime, qint64 endTime);

    void clear();

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;

    bool removeActivity(Activity* activity);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
private:
    void sortListItems();
    int addListItemsFor(Activity* activity);

    qint64 m_startTime = -1;
    qint64 m_endTime = -1;
    QVector<Activity*> m_activities;
    QVector<ActivityListItem> m_items;
};

#endif // ACTIVITYLISTMODEL_H
