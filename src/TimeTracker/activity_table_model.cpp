#include "activity_table_model.h"

ActivityTableModel::ActivityTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

QVariant ActivityTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical && role == Qt::DisplayRole)
    {
        return QString::number(section);
    }

    if (section == 0 && role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        return QStringLiteral("Name");
    }

    return QVariant();
}

//bool ActivityTableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
//{
//    if (value != headerData(section, orientation, role)) {
//        // FIXME: Implement me!
//        emit headerDataChanged(orientation, section, section);
//        return true;
//    }
//    return false;
//}


int ActivityTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_activities.count();
}

int ActivityTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 1;
}

QVariant ActivityTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int col = index.column();
    const Activity * activity = m_activities[row];

    if (role == Qt::DisplayRole)
    {
        return activity->name;
    }

    // FIXME: Implement me!
    return QVariant();
}

bool ActivityTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
//    if (data(index, role) != value) {
//        // FIXME: Implement me!
//        emit dataChanged(index, index, QVector<int>() << role);
//        return true;
//    }
    return false;
}

//Qt::ItemFlags ActivityTableModel::flags(const QModelIndex &index) const
//{
//    if (!index.isValid())
//        return Qt::NoItemFlags;

//    return Qt::ItemIsEditable; // FIXME: Implement me!
//}

void ActivityTableModel::setActivities(QVector<Activity *> activities)
{
    this->beginResetModel();
    m_activities = activities;
    this->endResetModel();
}
