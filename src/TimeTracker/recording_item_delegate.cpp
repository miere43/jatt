#include "recording_item_delegate.h"

#include <QStyleOptionViewItem>
#include <QPainter>
#include "common.h"

RecordingItemDelegate::RecordingItemDelegate(SessionRecorder* recorder, QObject *parent)
    : QStyledItemDelegate(parent)
{
    if (recorder != nullptr)
    {
        connect(recorder,  &SessionRecorder::recordStateChanged,
                this, &RecordingItemDelegate::recordStateChanged);
    }
    else
    {
        APP_ERRSTREAM << "recorder == nullptr";
    }
}

void RecordingItemDelegate::recordStateChanged(SessionRecorder *recorder, RecordingState state)
{
    if (state == RecordingState::Started)
    {
        Q_ASSERT(m_currentRecording == nullptr);
        m_currentRecording = recorder->recording();
    }
    else if (state == RecordingState::Finished)
    {
        Q_ASSERT(m_currentRecording == recorder->recording());
        m_currentRecording = nullptr;
    }
    else
    {
        Q_ASSERT(false);
    }

    //this->
}

void RecordingItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant recordingVariant = index.model()->data(index, Qt::UserRole);
    Recording* recording = nullptr;
    if (!recordingVariant.isValid() || (recording = (Recording*)recordingVariant.value<void*>()) == nullptr)
    {
        APP_ERRSTREAM << "invalid session received for UserRole";
        //intervalVariant = index.model()->data(index, Qt::DisplayRole);
        return;
    }

    if (recording == m_currentRecording)
    {
        painter->fillRect(option.rect, QColor(255, 0, 0));
    }
    else if (option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, option.palette.highlight());
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setFont(option.font);

    if (option.state & QStyle::State_Selected) {
        painter->setPen(QPen(option.palette.highlightedText(), 1));
    } else {
        painter->setPen(QPen(option.palette.text(), 1));
    }

    painter->drawText(option.rect, QString("%1 (%2 intervals)").arg(recording->name).arg(recording->intervals.count()));
    painter->restore();
}

QSize RecordingItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QSize(option.rect.width(), option.fontMetrics.height());
}
