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

    beginInsertRows(QModelIndex(), m_activities.count(), m_activities.count() + 4);
    m_activities.append(activity);
    endInsertRows();
}

void ActivityListModel::addActivities(QVector<Activity *> *activities)
{
    Q_ASSERT(activities);

    if (activities->count() > 0) {
        beginInsertRows(QModelIndex(), m_activities.count(), m_activities.count() + activities->count());
        for (Activity* a : *activities) {
            m_activities.append(a);
        }
        endInsertRows();
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
    beginRemoveRows(QModelIndex(), 0, m_activities.count());
    m_activities.clear();
    //m_items.clear();
    endRemoveRows();
}

int ActivityListModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return m_activities.count();
}

QVariant ActivityListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        return m_activities[index.row()]->displayString();
    }
    else if (role == Qt::UserRole)
    {
        // It's better to return ActivityListItem, but there is some code that relies
        // on returning object of type Activity.
        return QVariant::fromValue((void*)m_activities.at(index.row()));
    }

    return QVariant();
}

bool ActivityListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row + count > m_activities.count()) {
        APP_ERRSTREAM << "invalid params";
        return false;
    }

    beginRemoveRows(parent, row, row + count);
    m_activities.remove(row, count);
    endRemoveRows();

    return true;
}

bool ActivityListModel::removeActivity(Activity *activity) {
    Q_ASSERT(activity);

    int index = m_activities.indexOf(activity);
    if (index != -1) {
        beginRemoveRows(QModelIndex(), index, index);
        m_activities.remove(index);
        endRemoveRows();
        return true;
    }

    return false;
}

void ActivityListModel::dataChangedHint(Activity *activity) {
    Q_ASSERT(activity);
    int index = m_activities.indexOf(activity);
    Q_ASSERT(index != -1);
    emit dataChanged(this->index(index), this->index(index));
}
