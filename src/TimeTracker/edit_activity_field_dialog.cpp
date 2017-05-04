#include "edit_activity_field_dialog.h"
#include "ui_edit_activity_field_dialog.h"

EditActivityFieldDialog::EditActivityFieldDialog(Activity* activity, QString fieldName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditActivityFieldDialog)
{
    ui->setupUi(this);

    m_activity = activity;
    m_fieldName = fieldName;

    Q_ASSERT(m_activity);
//    Q_ASSERT(m_activity->id > 0);
    Q_ASSERT(m_activity->info);

    // @TODO: make sure field really exists.

    this->setupUi();
}

QVariant EditActivityFieldDialog::newValue() const {
    return ui->propertyLineEdit->text();
}

void EditActivityFieldDialog::setupUi()
{
    ui->fieldNameLabel->setText(QStringLiteral("Edit <b>") + m_fieldName +
                                QStringLiteral("</b> field of \"") + m_activity->displayString() + QStringLiteral("\"."));
    int i = m_activity->info->fieldNames.indexOf(m_fieldName);
    Q_ASSERT(i != -1);
    Q_ASSERT(m_activity->info->fieldNames.count() == m_activity->fieldValues.count());

    ui->propertyLineEdit->setText(m_activity->fieldValues[i]);
    ui->propertyLineEdit->selectAll();
}

EditActivityFieldDialog::~EditActivityFieldDialog()
{
    delete ui;
}
