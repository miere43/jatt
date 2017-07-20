#include "edit_activity_field_dialog.h"
#include "ui_edit_activity_field_dialog.h"
#include "error_macros.h"

EditActivityFieldDialog::EditActivityFieldDialog(Activity* activity, int fieldIndex, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditActivityFieldDialog)
{
//    ui->setupUi(this);

//    m_activity = activity;
//    m_fieldIndex = fieldIndex;

//    ERR_VERIFY_NULL(m_activity);
//    ERR_VERIFY_NULL(m_activity->info);

//    // @TODO: make sure field really exists.

//    this->setupUi();
}

QVariant EditActivityFieldDialog::newValue() const
{
    return ui->propertyLineEdit->text();
}

void EditActivityFieldDialog::setupUi()
{
//    QString fieldName = m_activity->info->fieldNames[m_fieldIndex];

//    ui->fieldNameLabel->setText(QStringLiteral("Edit <b>") + fieldName +
//                                QStringLiteral("</b> field of \"") + m_activity->displayString() + QStringLiteral("\"."));

//    ui->propertyLineEdit->setText(m_activity->field(m_fieldIndex));
//    ui->propertyLineEdit->selectAll();

//    ERR_VERIFY(m_activity->info->fieldNames.count() == m_activity->fieldValues.count());
}

EditActivityFieldDialog::~EditActivityFieldDialog()
{
    delete ui;
}
