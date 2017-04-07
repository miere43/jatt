#include "edit_recording_dialog.h"
#include "ui_edit_recording_dialog.h"

#include "application_state.h"

#include <QMessageBox>
#include <QDebug>

EditRecordingDialog::EditRecordingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditRecordingDialog)
{
    ui->setupUi(this);

    connect(this, &EditRecordingDialog::finished,
            this, &EditRecordingDialog::onDialogFinished);
}

void EditRecordingDialog::setRecording(Recording* recording)
{
    Q_ASSERT(recording);
    Q_ASSERT(m_recording == nullptr); // Recording change not supported yet.
    if (m_recording) return;

    m_recording = recording;
    populateFieldsFromRecordingData();
}

QString EditRecordingDialog::recordingName()
{
    Q_ASSERT(m_recording);
    Q_ASSERT(m_isValidationPassed);
    return ui->nameLineEdit->text();
}

QString EditRecordingDialog::recordingNote()
{
    Q_ASSERT(m_recording);
    Q_ASSERT(m_isValidationPassed);
    return ui->noteLineEdit->text();
}

QVector<Tag2*>* EditRecordingDialog::recordingTags()
{
    Q_ASSERT(m_recording);
    Q_ASSERT(m_isValidationPassed);
    return &m_validatedTags;
}

void EditRecordingDialog::populateFieldsFromRecordingData()
{
    Q_ASSERT(m_recording);
    ui->nameLineEdit->setText(m_recording->name);
    ui->noteLineEdit->setText(m_recording->note);
    QVector<Tag2*> recordingTags;
    if (!g_app.database()->loadTagsAssociatedWithRecording(m_recording, &recordingTags))
    {
        qCritical() << "just crash right now @TODO";
        QMessageBox::critical(this, "I just want to crash", "Yay!");
        QApplication::exit(1);
    }
    ui->tagsLineEdit->setText(tagsToString(recordingTags.data(), recordingTags.count()));
}

void EditRecordingDialog::onDialogFinished(int result)
{
    if (result != QDialog::Accepted)
    {
        return;
    }

    // validate
    m_isValidationPassed = true;

    QStringList tagNames = ui->tagsLineEdit->text().split(",", QString::SkipEmptyParts);
    for (const QString& tagName : tagNames)
    {
        Tag2* tag = g_app.database()->getTagByNameOrCreateIt(tagName);
        if (tag == nullptr)
        {
            m_isValidationPassed = false;
            break;
        }

        qDebug() << "did tag" << tag->name;
        m_validatedTags.append(tag);
    }
}

bool EditRecordingDialog::isValidationPassed() const
{
    return m_isValidationPassed;
}

Recording* EditRecordingDialog::recording() const
{
    return m_recording;
}

EditRecordingDialog::~EditRecordingDialog()
{
    delete ui;
}
