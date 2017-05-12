#include "edit_activity_field_dialog.h"
#include "ui_edit_activity_field_dialog.h"

EditActivityFieldDialog::EditActivityFieldDialog(Activity* activity, int fieldIndex, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditActivityFieldDialog)
{
    ui->setupUi(this);

    m_activity = activity;
    m_fieldIndex = fieldIndex;

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
    QString fieldName = m_activity->info->fieldNames[m_fieldIndex];

    ui->fieldNameLabel->setText(QStringLiteral("Edit <b>") + fieldName +
                                QStringLiteral("</b> field of \"") + m_activity->displayString() + QStringLiteral("\"."));
    Q_ASSERT(m_activity->info->fieldNames.count() == m_activity->fieldValues.count());

    ui->propertyLineEdit->setText(m_activity->field(m_fieldIndex));
    ui->propertyLineEdit->selectAll();
}

EditActivityFieldDialog::~EditActivityFieldDialog()
{
    delete ui;
}
