#include "edit_activity_field_dialog.h"
#include "ui_edit_activity_field_dialog.h"
#include "error_macros.h"

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
}

QString EditActivityFieldDialog::newName() const
{
    return ui->nameEdit->text();
}

QString EditActivityFieldDialog::newNote() const
{
    return ui->noteEdit->toPlainText();
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

EditActivityFieldDialog::~EditActivityFieldDialog()
{
    delete ui;
}
