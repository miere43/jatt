#include "activity_list_model.h"
#include "common.h"
#include <QtAlgorithms>

ActivityListModel::ActivityListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void ActivityListModel::addActivity(Activity *activity)
{
    Q_ASSERT(activity);
    Q_ASSERT(activity->id != -1);
    Q_ASSERT(activity->info != nullptr);

    m_activities.append(activity);
    int itemsCount = addListItemsFor(activity);

    if (itemsCount > 0) {
        beginInsertRows(QModelIndex(), m_items.count(), itemsCount + m_items.count());
        endInsertRows();

        sortListItems();
    }
}

void ActivityListModel::addActivities(QVector<Activity *> *activities)
{
    Q_ASSERT(activities);

    int addedItemsCount = 0;
    for (Activity* activity : *activities)
    {
        Q_ASSERT(activity);
        Q_ASSERT(activity->id != -1);
        Q_ASSERT(activity->info != nullptr);

        m_activities.append(activity);
        addedItemsCount += addListItemsFor(activity);
    }

    if (addedItemsCount > 0) {
        int itemsCountInitial = m_items.count();
        beginInsertRows(QModelIndex(), itemsCountInitial, itemsCountInitial + addedItemsCount);
        endInsertRows();

        sortListItems();
    }
}

void ActivityListModel::setTimePeriod(qint64 startTime, qint64 endTime)
{
    m_startTime = startTime;
    m_endTime = endTime;
    // @TODO: invalidate items.
}

void ActivityListModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, m_items.count());
    m_activities.clear();
    m_items.clear();
    endRemoveRows();
}

int ActivityListModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return m_items.count();
}

QVariant ActivityListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        return m_items[index.row()].activity->displayString();
    }
    else if (role == Qt::UserRole)
    {
        // It's better to return ActivityListItem, but there is some code that relies
        // on returning object of type Activity.
        return QVariant::fromValue((void*)&m_items.at(index.row()));
    }

    return QVariant();
}

bool ActivityListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row + count > m_items.count()) {
        APP_ERRSTREAM << "invalid params";
        return false;
    }

    beginRemoveRows(parent, row, row + count);
    m_items.remove(row, count);
    endRemoveRows();

    return true;
}

int ActivityListModel::addListItemsFor(Activity *activity)
{
    int count = 0;
    for (const Interval& interval : activity->intervals)
    {
        if (interval.isInTimePeriod(m_startTime, m_endTime)) {
            ActivityListItem obj;
            obj.interval = &interval;
            obj.activity = activity;
            m_items.append(obj);
            ++count;
        }
    }
    return count;
}

void ActivityListModel::sortListItems()
{
    qSort(m_items.begin(), m_items.end());
    emit dataChanged(QModelIndex(), QModelIndex());
}

void ActivityListModel::addActivityInterval(Activity *activity, const Interval *interval, bool forceSort) {
    Q_ASSERT(activity);
    Q_ASSERT(interval);

#ifdef QT_DEBUG
    Q_ASSERT(m_activities.contains(activity));
#endif

    ActivityListItem i;
    i.activity = activity;
    i.interval = interval;

    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
    m_items.append(i);
    endInsertRows();

    if (forceSort)
        sortListItems();
}

bool ActivityListModel::removeActivity(Activity *activity) {
    Q_ASSERT(activity);
    int itemsLeft = activity->intervals.count();
    if (itemsLeft == 0) itemsLeft = 1; // if there are no intervals, there is ListItem for activity itself.

    // @TODO: if we remove activity, we don't need to sort ListItems again, right?
    for (int i = 0; i < m_items.count(); ++i) {
        if (m_items.at(i).activity == activity) {
            // @TODO: batch removeRows calls.
            beginRemoveRows(QModelIndex(), i, i);
            m_items.remove(i--);
            endRemoveRows();
            if ((--itemsLeft) <= 0) {
                // we have one ListItem per Interval, if there are no intervals, there is nothing to delete.
                return true;
            }
        }
    }

    Q_ASSERT(itemsLeft == 0);
    return true;
}

bool operator<(const ActivityListItem& lhs, const ActivityListItem& rhs) {
    if (lhs.interval == nullptr) return false;
    if (rhs.interval == nullptr) return true;
    return lhs.interval->startTime < rhs.interval->startTime;
}
