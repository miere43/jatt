#ifndef RECORDING_LIST_MODEL_H
#define RECORDING_LIST_MODEL_H

#include "core_types.h"
#include <QAbstractListModel>
#include <QList>

struct Session;

class RecordingListModel : public QAbstractListModel
{
public:
    QVector<Recording*> m_recordings;

    RecordingListModel();

    void appendRecording(Recording* recording);
    void sendDataChangedHint(Recording* recording);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const;
};

#endif // RECORDING_LIST_MODEL_H
