#ifndef ACTIVITYLISTMODEL_H
#define ACTIVITYLISTMODEL_H

#include <QAbstractListModel>

#include "core_types.h"

class ActivityListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ActivityListModel(QObject * parent = 0);

    QModelIndex addActivity(Activity * activity);
    void addActivities(const QVector<Activity *> & activities);

    void setTimePeriod(qint64 startTime, qint64 endTime);

    void clear();

    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    bool removeRows(int row, int count, const QModelIndex & parent) override;

    bool removeActivity(Activity * activity);
    void dataChangedHint(Activity * activity);
    inline QVector<Activity *> & activities() { return m_activities; }

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
private:
    void sort();

    qint64 m_startTime = -1;
    qint64 m_endTime   = -1;
    QVector<Activity*> m_activities;
};

#endif // ACTIVITYLISTMODEL_H
