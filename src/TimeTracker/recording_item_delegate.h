#ifndef RECORDING_ITEM_DELEGATE_H
#define RECORDING_ITEM_DELEGATE_H

#include <QStyledItemDelegate>
#include "core_types.h"
#include "session_recorder.h"

class RecordingItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    RecordingItemDelegate(SessionRecorder* recorder, QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
private slots:
    void recordStateChanged(SessionRecorder* recorder, RecordingState state);
private:
    Recording* m_currentRecording = nullptr;
};

#endif // RECORDING_ITEM_DELEGATE_H
