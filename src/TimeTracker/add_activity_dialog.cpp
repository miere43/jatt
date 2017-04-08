#include "add_activity_dialog.h"
#include "ui_add_activity_dialog.h"

#include "add_field_dialog.h"
#include "application_state.h"

#include <QMessageBox>
#include <QLineEdit>
#include <QDebug>
#include <QLabel>

AddActivityDialog::AddActivityDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddActivityDialog)
{
    ui->setupUi(this);

    Q_ASSERT(g_app.database()->loadActivityInfos());
    QList<ActivityInfo*> infos = g_app.database()->activityInfos();

    if (infos.count() == 0)
    {
        QMessageBox::critical(this, "Error", "No activity infos present.");
        close();
        return;
    }

    for (const ActivityInfo* info : infos)
    {
        ui->activityInfoComboBox->addItem(info->name, QVariant::fromValue((void*)info));
    }

    ui->activityInfoComboBox->setCurrentIndex(0);
    ui->scrollArea->setEnabled(false);
    setActivityInfo(infos[0]);
}

AddActivityDialog::~AddActivityDialog()
{
    delete ui;
}

void AddActivityDialog::setActivityInfo(ActivityInfo* info)
{
    Q_ASSERT(info);

    if (m_currentActivityInfo)
    {
        for (auto widget : m_activityInfoWidgets)
        {
            delete widget;
        }
        m_activityInfoWidgets.clear();
        m_activityInfoFieldWidgets.clear();
    }

    m_currentActivityInfo = info;
    ui->scrollArea->setEnabled(true);

    for (const QString& field : m_currentActivityInfo->fieldNames)
    {
        QLayout* layout = ui->verticalLayout_3;
        QLabel* label = new QLabel();
        label->setText(field);
        QLineEdit* lineEdit = new QLineEdit();
        layout->addWidget(label);
        layout->addWidget(lineEdit);
        m_activityInfoWidgets.append(label);
        m_activityInfoWidgets.append(lineEdit);
        m_activityInfoFieldWidgets.append(lineEdit);
    }

    qDebug() << "Activity Info changed to" << m_currentActivityInfo->name;
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
    Q_ASSERT(dialog);

    setActivityInfo(m_currentActivityInfo);
}

void AddActivityDialog::addFieldDialogFinished(int result)
{
    Q_UNUSED(result);

    QObject* sender = QObject::sender();
    AddFieldDialog* dialog = qobject_cast<AddFieldDialog*>(sender);
    Q_ASSERT(dialog);

    dialog->deleteLater();
}

void AddActivityDialog::on_activityInfoComboBox_currentIndexChanged(int index)
{
    ActivityInfo* activityInfo = (ActivityInfo*)ui->activityInfoComboBox->itemData(index, Qt::UserRole).value<void*>();
    Q_ASSERT(activityInfo);

    setActivityInfo(activityInfo);
}

Activity* AddActivityDialog::constructActivity()
{
    Q_ASSERT(!m_activityWasConstructed); // call this thing only once
    if (!m_currentActivityInfo)
        return nullptr;
    if (m_activityWasConstructed)
        return nullptr;

    Activity* activity = g_app.m_activityAllocator.allocate();
    Q_ASSERT(activity);

    activity->id = -1;
    activity->info = m_currentActivityInfo;
    activity->startTime = getCurrentDateTimeUtc();
    activity->endTime = activity->startTime;
    activity->fieldValues.reserve(m_activityInfoFieldWidgets.size());

    for (const QWidget* fieldWidget : m_activityInfoFieldWidgets)
    {
        const QLineEdit* lineEdit = qobject_cast<const QLineEdit*>(fieldWidget);
        Q_ASSERT(lineEdit);

        activity->fieldValues.append(lineEdit->text());
    }

    Q_ASSERT(g_app.database()->saveActivity(activity));
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
