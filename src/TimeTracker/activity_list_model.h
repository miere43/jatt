#ifndef ACTIVITYLISTMODEL_H
#define ACTIVITYLISTMODEL_H

#include <QAbstractListModel>

#include "core_types.h"

class ActivityListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ActivityListModel(QObject *parent = 0);

    void addActivity(Activity* activity);
    void addActivities(QVector<Activity*>* activities);

    void clear();

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
private:
    QVector<Activity*> m_activities;
};

#endif // ACTIVITYLISTMODEL_H
