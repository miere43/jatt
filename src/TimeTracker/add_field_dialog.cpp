#include "add_field_dialog.h"
#include "ui_add_field_dialog.h"

#include "utilities.h"

AddFieldDialog::AddFieldDialog(ActivityInfo* info, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddFieldDialog),
    m_activityInfo(info)
{
    ui->setupUi(this);
}

AddFieldDialog::~AddFieldDialog()
{
    delete ui;
}

void AddFieldDialog::on_buttonBox_accepted()
{
    //addFieldToActivityInfo(m_activityInfo, ui->fieldNameLineEdit->text(),  /* @TODO */ ActivityInfoFieldType::String);
    //emit fieldAdded();
}
