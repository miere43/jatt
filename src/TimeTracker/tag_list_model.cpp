#include "tag_list_model.h"

TagListModel::TagListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void TagListModel::addTag(Tag2 *tag)
{
    Q_ASSERT(tag);
    m_tags.append(tag);
}

Tag2* TagListModel::tagAtRow(int row) const
{
    if (row < 0 || row >= m_tags.count())
        return nullptr;
    return m_tags[row];
}

//QVariant TagListModel::headerData(int section, Qt::Orientation orientation, int role) const
//{
//    // FIXME: Implement me!
//}

int TagListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_tags.count();
}

QVariant TagListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int pos = index.row();
    if (pos < 0 || pos >= m_tags.count())
        return QVariant();

    Tag2* tag = m_tags[pos];
    if (role == Qt::DisplayRole)
    {
        return QVariant(tag->name);
    }

    return QVariant();
}
