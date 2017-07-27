#include "edit_activity_field_dialog.h"
#include "ui_edit_activity_field_dialog.h"
#include "error_macros.h"
#include "application_state.h"

#include <QDebug>

EditActivityFieldDialog::EditActivityFieldDialog(Activity* activity, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditActivityFieldDialog)
{
    ui->setupUi(this);

    m_activity = activity;

    ERR_VERIFY_NULL(m_activity);
    ERR_VERIFY_NULL(m_activity->info);

    ui->nameEdit->setText(activity->name);
    ui->nameEdit->selectAll();
    ui->noteEdit->setText(activity->note);

    QList<ActivityInfo*> infos = g_app.database()->activityInfos();

    int i = 0;
    for (const ActivityInfo* info : infos)
    {
        ui->typeComboBox->addItem(info->name, QVariant::fromValue<void*>((void*)info));
        if (info == activity->info) {
            ui->typeComboBox->setCurrentIndex(i);
        }
        ++i;
    }
}

QString EditActivityFieldDialog::newName() const
{
    return ui->nameEdit->text();
}

QString EditActivityFieldDialog::newNote() const
{
    return ui->noteEdit->toPlainText();
}

ActivityInfo * EditActivityFieldDialog::newActivityInfo() const
{
    QVariant infoVariant = ui->typeComboBox->currentData();
    qDebug() << "type:" << infoVariant.type();
    if (infoVariant.isValid())
    {
        return (ActivityInfo*)infoVariant.value<void*>();

    }
    else
    {
        return m_activity->info;
    }
}

bool EditActivityFieldDialog::isNameFieldChanged() const
{
    return ui->nameEdit->isModified();
}

bool EditActivityFieldDialog::isNoteFieldChanged() const
{
    QTextDocument * document = ui->noteEdit->document();
    ERR_VERIFY_NULL_V(document, true);

    return document->isModified();
}

bool EditActivityFieldDialog::isActivityInfoChanged() const
{
    QVariant infoVariant = ui->typeComboBox->currentData();
    if (!infoVariant.isValid()) return false;
    ActivityInfo* info = (ActivityInfo*)infoVariant.value<void*>();
    return info != m_activity->info;
}

EditActivityFieldDialog::~EditActivityFieldDialog()
{
    delete ui;
}
