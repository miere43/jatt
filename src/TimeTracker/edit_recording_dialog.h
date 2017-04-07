#ifndef EDIT_RECORDING_DIALOG_H
#define EDIT_RECORDING_DIALOG_H

#include "core_types.h"

#include <QDialog>

namespace Ui {
class EditRecordingDialog;
}

class EditRecordingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditRecordingDialog(QWidget *parent = 0);
    ~EditRecordingDialog();

    void setRecording(Recording* session);
    Recording* recording() const;

    QString recordingName();
    QString recordingNote();
    QVector<Tag2*>* recordingTags();

    bool isValidationPassed() const;
private slots:
    void onDialogFinished(int result);
private:
    void populateFieldsFromRecordingData();

    Ui::EditRecordingDialog *ui = nullptr;

    QVector<Tag2*> m_validatedTags;
    bool m_isValidationPassed = false;
    Recording* m_recording = nullptr;
};

#endif // EDIT_RECORDING_DIALOG_H
