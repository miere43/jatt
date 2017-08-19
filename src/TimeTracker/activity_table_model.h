#ifndef ACTIVITY_TABLE_MODEL_H
#define ACTIVITY_TABLE_MODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include "core_types.h"

class ActivityTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ActivityTableModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

//    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

//    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void setActivities(QVector<Activity *> activities);
private:
    QVector<Activity *> m_activities;
};

#endif // ACTIVITY_TABLE_MODEL_H
