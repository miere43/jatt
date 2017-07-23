#include "edit_activity_field_dialog.h"
#include "ui_edit_activity_field_dialog.h"
#include "error_macros.h"


EditActivityFieldDialog::EditActivityFieldDialog(Activity* activity, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditActivityFieldDialog)
{
    ui->setupUi(this);

    m_activity = activity;

    ERR_VERIFY_NULL(m_activity);
    ERR_VERIFY_NULL(m_activity->info);

    m_isNameFieldChanged = true;
    m_isNoteFieldChanged = true; // @TODO @TEMP

    ui->nameEdit->setText(activity->name);
    ui->nameEdit->selectAll();
    ui->noteEdit->setPlainText(activity->note);
}

QString EditActivityFieldDialog::newName() const
{
    return ui->nameEdit->text();
}

QString EditActivityFieldDialog::newNote() const
{
    return ui->noteEdit->toPlainText();
}

EditActivityFieldDialog::~EditActivityFieldDialog()
{
    delete ui;
}
