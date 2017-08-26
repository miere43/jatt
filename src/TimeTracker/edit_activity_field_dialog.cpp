#include "edit_activity_field_dialog.h"
#include "ui_edit_activity_field_dialog.h"
#include "error_macros.h"
#include "application_state.h"

#include <QDebug>

EditActivityFieldDialog::EditActivityFieldDialog(Activity * activity, QWidget * parent) :
    QDialog(parent),
    ui(new Ui::EditActivityFieldDialog)
{
    ui->setupUi(this);

    m_activity = activity;

    ERR_VERIFY_NULL(m_activity);
    ERR_VERIFY_NULL(m_activity->category);

    ui->nameEdit->setText(activity->name);
    ui->nameEdit->selectAll();
    ui->noteEdit->setPlainText(activity->note);

    QList<ActivityCategory *> categories = g_app.database()->activityCategories();

    int i = 0;
    for (const ActivityCategory * category : categories)
    {
        ui->typeComboBox->addItem(category->name, QVariant::fromValue<void*>((void*)category));
        if (category == activity->category) {
            ui->typeComboBox->setCurrentIndex(i);
        }
        ++i;
    }
}

QString EditActivityFieldDialog::newName() const
{
    return ui->nameEdit->text();
}

QString EditActivityFieldDialog::newNote() const
{
    return ui->noteEdit->toPlainText();
}

ActivityCategory * EditActivityFieldDialog::newActivityCategory() const
{
    QVariant categoryVariant = ui->typeComboBox->currentData();

    if (categoryVariant.isValid())
    {
        return (ActivityCategory *)categoryVariant.value<void *>();
    }
    else
    {
        return m_activity->category;
    }
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

bool EditActivityFieldDialog::isActivityCategoryChanged() const
{
    QVariant categoryVariant = ui->typeComboBox->currentData();
    if (!categoryVariant.isValid()) return false;
    ActivityCategory * category = (ActivityCategory *)categoryVariant.value<void *>();
    return category != m_activity->category;
}

EditActivityFieldDialog::~EditActivityFieldDialog()
{
    delete ui;
}
