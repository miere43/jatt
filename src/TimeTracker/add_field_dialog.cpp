#include "add_field_dialog.h"
#include "ui_add_field_dialog.h"

#include "utilities.h"

AddFieldDialog::AddFieldDialog(ActivityCategory* category, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddFieldDialog),
    m_activityCategory(category)
{
    ui->setupUi(this);
}

AddFieldDialog::~AddFieldDialog()
{
    delete ui;
}

void AddFieldDialog::on_buttonBox_accepted()
{

}
