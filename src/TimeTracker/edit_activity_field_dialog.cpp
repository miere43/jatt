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

void EditActivityFieldDialog::setupUi() {
    ui->fieldNameLabel->setText(QStringLiteral("Edit <b>") + m_fieldName + QStringLiteral("</b>"));
}

EditActivityFieldDialog::~EditActivityFieldDialog()
{
    delete ui;
}
