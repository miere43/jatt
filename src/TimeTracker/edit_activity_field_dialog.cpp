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
    ui->nameEdit->setFocus();
    ui->noteEdit->setPlainText(activity->note);
    ui->favoriteCheckBox->setCheckState(activity->favorite ? Qt::Checked : Qt::Unchecked);

    QList<ActivityCategory *> categories = g_app.database()->activityCategories();

    int i = 0;
    ui->typeComboBox->blockSignals(true); // Prevent "updatePrevActivityName" from firing.
    for (ActivityCategory * category : categories)
    {
        ui->typeComboBox->addItem(category->name, QVariant::fromValue<void*>(static_cast<void*>(category)));
        if (category == activity->category) {
            ui->typeComboBox->blockSignals(false);
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
        return static_cast<ActivityCategory*>(categoryVariant.value<void *>());
    }
    else
    {
        return m_activity->category;
    }
}

bool EditActivityFieldDialog::newFavorite() const
{
    return ui->favoriteCheckBox->checkState() == Qt::Checked;
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
    auto category = static_cast<ActivityCategory *>(categoryVariant.value<void *>());
    return category != m_activity->category;
}

bool EditActivityFieldDialog::isFavoriteChanged() const
{
    auto favorite = ui->favoriteCheckBox->checkState() == Qt::Checked;
    return favorite != m_activity->favorite;
}

EditActivityFieldDialog::~EditActivityFieldDialog()
{
    delete ui;
}

void EditActivityFieldDialog::on_prevActivityLabel_linkActivated(const QString &link)
{
    Q_UNUSED(link);
    ui->nameEdit->setText(m_prevActivityName);
    ui->nameEdit->setFocus(Qt::OtherFocusReason);
    ui->nameEdit->setModified(true);
}

void EditActivityFieldDialog::on_typeComboBox_currentIndexChanged(int index)
{
    auto currentData = ui->typeComboBox->itemData(index);
    if (currentData.isValid())
    {
        auto category = static_cast<ActivityCategory*>(currentData.value<void*>());
        ERR_VERIFY(category);
        qDebug() << category->name;
        updatePrevActivityName(category);
    }
}

void EditActivityFieldDialog::updatePrevActivityName(ActivityCategory * category)
{
    m_prevActivityName = g_app.database()->loadFirstActivityNameBeforeEndTime(category, m_activity->endTime);
    if (m_prevActivityName.isEmpty() || m_prevActivityName == m_activity->name)
    {
        ui->prevActivityLabel->hide();
    }
    else
    {
        ui->prevActivityLabel->show();

        // Truncate very long text
        QFontMetrics metrics(ui->prevActivityLabel->font());
        QString elidedText = metrics.elidedText(
            "<a href=\"#\">" + m_prevActivityName.toHtmlEscaped() + "</a>",
            Qt::ElideRight,
            ui->prevActivityLabel->parentWidget()->width());
        ui->prevActivityLabel->setText(elidedText);
    }
}
