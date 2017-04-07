#include "add_activity_dialog.h"
#include "ui_add_activity_dialog.h"

AddActivityDialog::AddActivityDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddActivityDialog)
{
    ui->setupUi(this);
}

AddActivityDialog::~AddActivityDialog()
{
    delete ui;
}
