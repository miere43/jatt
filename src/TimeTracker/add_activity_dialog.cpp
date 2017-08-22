#include "add_activity_dialog.h"
#include "ui_add_activity_dialog.h"

#include "add_activity_info_dialog.h"
#include "add_field_dialog.h"
#include "application_state.h"
#include "error_macros.h"

#include <QMessageBox>
#include <QLineEdit>
#include <QDebug>
#include <QLabel>

AddActivityDialog::AddActivityDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddActivityDialog)
{
    ui->setupUi(this);

    g_app.database()->loadActivityCategories();
    QList<ActivityCategory*> categories = g_app.database()->activityCategories();

    if (categories.count() == 0)
    {
        QMessageBox::critical(this, "Error", "No categories are present.");
        close();
        return;
    }

    for (const ActivityCategory* category : categories)
    {
        ui->activityInfoComboBox->addItem(category->name, QVariant::fromValue((void*)category));
    }

    ui->activityInfoComboBox->setCurrentIndex(0);
    ui->scrollArea->setEnabled(false);

    setActivityCategory(categories[0]);
}

AddActivityDialog::~AddActivityDialog()
{
    delete ui;
}

void AddActivityDialog::setActivityCategory(ActivityCategory * category)
{
    ERR_VERIFY_NULL(category);

    if (m_currentActivityInfo)
    {
        for (auto widget : m_activityInfoWidgets)
        {
            delete widget;
        }
        m_activityInfoWidgets.clear();
        m_activityInfoFieldWidgets.clear();
    }

    m_currentActivityInfo = category;
    ui->scrollArea->setEnabled(true);
}

void AddActivityDialog::on_addFieldButton_clicked()
{
    AddFieldDialog* dialog = new AddFieldDialog(m_currentActivityInfo, this);
    connect(dialog, &AddFieldDialog::fieldAdded,
            this, &AddActivityDialog::addFieldDialogFieldAdded);
    connect(dialog, &AddFieldDialog::finished,
            this, &AddActivityDialog::addFieldDialogFinished);
    dialog->exec();
}

void AddActivityDialog::addFieldDialogFieldAdded()
{
    QObject* sender = QObject::sender();
    AddFieldDialog* dialog = qobject_cast<AddFieldDialog*>(sender);
    ERR_VERIFY_NULL(dialog);

    setActivityCategory(m_currentActivityInfo);
}

void AddActivityDialog::addFieldDialogFinished(int result)
{
    Q_UNUSED(result);

    QObject* sender = QObject::sender();
    AddFieldDialog* dialog = qobject_cast<AddFieldDialog*>(sender);
    ERR_VERIFY_NULL(dialog);

    dialog->deleteLater();
}

void AddActivityDialog::on_activityInfoComboBox_currentIndexChanged(int index)
{
    ActivityCategory* activityInfo = (ActivityCategory*)ui->activityInfoComboBox->itemData(index, Qt::UserRole).value<void*>();
    ERR_VERIFY_NULL(activityInfo);

    setActivityCategory(activityInfo);
}

Activity* AddActivityDialog::constructActivity()
{
    ERR_VERIFY_V(!m_activityWasConstructed, nullptr); // call this thing only once

    if (!m_currentActivityInfo)
        return nullptr;
    if (m_activityWasConstructed)
        return nullptr;

    Activity* activity = g_app.m_activityAllocator.allocate();
    ERR_VERIFY_NULL_V(activity, nullptr);

    activity->id = -1;
    activity->category = m_currentActivityInfo;
    activity->startTime = getCurrentDateTimeUtc();
    activity->endTime = activity->startTime;

//    activity->fieldValues.reserve(m_activityInfoFieldWidgets.size());

//    for (const QWidget* fieldWidget : m_activityInfoFieldWidgets)
//    {
//        const QLineEdit* lineEdit = qobject_cast<const QLineEdit*>(fieldWidget);
//        if (!lineEdit) {
//            g_app.m_activityAllocator.deallocate(activity);
//            ERR_VERIFY_NULL_V(lineEdit, nullptr);
//        }

//        activity->fieldValues.append(lineEdit->text());
//    }

    g_app.database()->saveActivity(activity);
    m_activityWasConstructed = true;

    return activity;
}

void AddActivityDialog::on_AddActivityDialog_finished(int result)
{
    Q_UNUSED(result);
}

void AddActivityDialog::on_buttonBox_accepted()
{
    this->accept();
}

void AddActivityDialog::on_buttonBox_rejected()
{
    this->reject();
}

void AddActivityDialog::on_addActivityInfo_clicked()
{
    AddActivityInfoDialog* dialog = new AddActivityInfoDialog();
    connect(dialog, &AddActivityInfoDialog::finished,
            this, &AddActivityDialog::addActivityInfoDialogFinished);
    connect(dialog, &AddActivityInfoDialog::infoAdded,
            this, &AddActivityDialog::addActivityInfoDialogInfoAdded);

    dialog->exec();
}

void AddActivityDialog::addActivityInfoDialogInfoAdded()
{
    QObject* sender = QObject::sender();
    AddActivityInfoDialog* dialog = qobject_cast<AddActivityInfoDialog*>(sender);
    ERR_VERIFY(dialog);

    ActivityCategory* info = dialog->constructActivityInfo();
    ERR_VERIFY(info);

    ui->activityInfoComboBox->addItem(info->name, QVariant::fromValue((void*)info));
    ui->activityInfoComboBox->setCurrentIndex(ui->activityInfoComboBox->children().count() - 1);
}

void AddActivityDialog::addActivityInfoDialogFinished(int result)
{
    Q_UNUSED(result);

    QObject* sender = QObject::sender();
    AddActivityInfoDialog* dialog = qobject_cast<AddActivityInfoDialog*>(sender);
    ERR_VERIFY(dialog);

    dialog->deleteLater();
}

