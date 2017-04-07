#include "recording_list_model.h"

#include <QVariant>
#include "common.h"

RecordingListModel::RecordingListModel()
{
}

int RecordingListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() != 0) {
        return m_recordings.count();
    } else {
        APP_ERRSTREAM << "- parent.column() =" << parent.column();
        return 0;
    }
}

QVariant RecordingListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        if (index.column() != 0) {
            APP_ERRSTREAM << "- index.column() =" << index.row();
            return QVariant();
        }
        if (index.row() < 0 || index.row() > m_recordings.count()) {
            APP_ERRSTREAM << "- index.row() =" << index.row();
            return QVariant();
        }
        Recording* recording = m_recordings.at(index.row());
        return QVariant(recording->name + "(" + QString::number(recording->intervals.count()) + ")");
    }
    else if (role == Qt::UserRole) {
        // UserRole returns pointer to recording at specified row.
        if (index.row() < 0 || index.row() > m_recordings.count()) {
            APP_ERRSTREAM << "- get UserRole row out of bounds:" << index.row();
            return QVariant();
        }
        Recording* recording = m_recordings[index.row()];
        return qVariantFromValue((void*)recording);
    } else {
        return QVariant();
    }
}

void RecordingListModel::appendRecording(Recording *recording)
{
    if (recording == nullptr)
    {
        APP_ERRSTREAM << "recording == nullptr";
        return;
    }

    QModelIndex index = QModelIndex();
    int count = rowCount(index);
    this->beginInsertRows(index, count, count);

    m_recordings.append(recording);

    this->endInsertRows();
}

void RecordingListModel::sendDataChangedHint(Recording *recording)
{
    if (recording == nullptr)
    {
        APP_ERRSTREAM << "recording == nullptr";
        return;
    }

    for (int i = 0; i < m_recordings.count(); ++i)
    {
        if (recording == m_recordings[i])
        {
            QModelIndex index = this->index(i);
            emit dataChanged(index, index, { Qt::DisplayRole });
            return;
        }
    }

    APP_ERRSTREAM << "recording not found:" << recording->name;
}
