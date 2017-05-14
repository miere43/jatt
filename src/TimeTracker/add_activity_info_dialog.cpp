#include "add_activity_info_dialog.h"
#include "ui_add_activity_info_dialog.h"

#include "application_state.h"
#include "error_macros.h"

#include <QMessageBox>
#include <QColorDialog>

AddActivityInfoDialog::AddActivityInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddActivityInfoDialog)
{
    ui->setupUi(this);
}

AddActivityInfoDialog::~AddActivityInfoDialog()
{
    delete ui;
}

ActivityInfo* AddActivityInfoDialog::constructActivityInfo()
{
    ERR_VERIFY_V(!m_activityInfoConstructed, nullptr);
    ERR_VERIFY_V(m_dialogAccepted, nullptr);

    ActivityInfo* info = g_app.m_activityInfoAllocator.allocate();
    ERR_VERIFY_NULL_V(info, nullptr);

    info->id = -1;
    info->name = ui->nameLineEdit->text();
    info->displayFormat = ui->displayFormatLineEdit->text();
    info->displayRules = "";
    info->color = m_selectedColor;

    g_app.database()->saveActivityInfo(info);

    info->updateFormatter();
    m_activityInfoConstructed = true;
    return info;
}

bool AddActivityInfoDialog::validate(QString* errorMessage)
{
    ERR_VERIFY_NULL_V(errorMessage, false);

    QString name = ui->nameLineEdit->text();
    if (name.isNull() || name.isEmpty())
    {
        *errorMessage = QLatin1Literal("Specify name.");
        return false;
    }

    g_app.database()->loadActivityInfos();
    for (const ActivityInfo* activityInfo : g_app.database()->activityInfos())
    {
        if (activityInfo->name.compare(name, Qt::CaseInsensitive) == 0)
        {
            *errorMessage = QLatin1Literal("Activity Template with name '") + name + QLatin1Literal("' already exists.");
            return false;
        }
    }

    if (m_selectedColor < 0x00000000 || m_selectedColor > 0xFFFFFFFF)
    {
        *errorMessage = QLatin1Literal("Invalid color.");
        return false;
    }

    return true;
}

void AddActivityInfoDialog::on_buttonBox_accepted()
{
    QString errorMessage;
    if (!validate(&errorMessage))
    {
        QMessageBox::critical(this, "Validation Error", errorMessage);
        return;
    }

    m_dialogAccepted = true;
    this->accept();
    emit infoAdded();
}

void AddActivityInfoDialog::on_buttonBox_rejected()
{
    this->reject();
}

void AddActivityInfoDialog::on_pickColorButton_clicked()
{
    QColor color = QColorDialog::getColor(QColor((QRgb)m_selectedColor), this, "Select Color");
    if (!color.isValid())
    {
        m_selectedColor = 0;
    }

    m_selectedColor = color.rgb();
    ui->pickColorButton->setText(color.name());
}
