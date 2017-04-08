#include "activity_list_model.h"

ActivityListModel::ActivityListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void ActivityListModel::addActivity(Activity *activity)
{
    Q_ASSERT(activity);
    Q_ASSERT(activity->id != -1);
    Q_ASSERT(activity->info != nullptr);

    beginInsertRows(QModelIndex(), m_activities.count(), m_activities.count());
    m_activities.append(activity);
    endInsertRows();
}

void ActivityListModel::addActivities(QVector<Activity *> *activities)
{
    Q_ASSERT(activities);

    int count = m_activities.count();
    beginInsertRows(QModelIndex(), count, count + activities->count());
    for (Activity* activity : *activities)
    {
        m_activities.append(activity);
    }
    endInsertRows();
}

void ActivityListModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, m_activities.count());
    m_activities.clear();
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
        return QVariant::fromValue((void*)m_activities[index.row()]);
    }

    return QVariant();
}
