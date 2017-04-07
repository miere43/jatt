#include "edit_tags_dialog.h"
#include "ui_edit_tags_dialog.h"

EditTagsDialog::EditTagsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditTagsDialog)
{
    ui->setupUi(this);
}

EditTagsDialog::~EditTagsDialog()
{
    delete ui;
}
