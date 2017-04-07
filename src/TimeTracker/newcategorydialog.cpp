#include "newcategorydialog.h"
#include "ui_newcategorydialog.h"

NewCategoryDialog::NewCategoryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewCategoryDialog)
{
    ui->setupUi(this);
}

NewCategoryDialog::~NewCategoryDialog()
{
    delete ui;
}
