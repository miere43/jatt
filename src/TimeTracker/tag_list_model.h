#ifndef TAGLISTMODEL_H
#define TAGLISTMODEL_H

#include "core_types.h"

#include <QAbstractListModel>

class TagListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit TagListModel(QObject *parent = 0);

    void addTag(Tag2* tag);
    Tag2* tagAtRow(int row) const;

//    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QVector<Tag2*> m_tags;
};

#endif // TAGLISTMODEL_H
