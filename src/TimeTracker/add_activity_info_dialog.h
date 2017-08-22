#ifndef ADD_ACTIVITY_INFO_DIALOG_H
#define ADD_ACTIVITY_INFO_DIALOG_H

#include <QDialog>

#include "core_types.h"

namespace Ui {
class AddActivityInfoDialog;
}

class AddActivityInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddActivityInfoDialog(QWidget *parent = 0);
    ~AddActivityInfoDialog();

    ActivityCategory* constructActivityInfo();
signals:
    void infoAdded();
private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_pickColorButton_clicked();
private:
    bool validate(QString* errorMessage);

    Ui::AddActivityInfoDialog *ui;

    qint64 m_selectedColor = 0x000000;
    bool m_activityInfoConstructed = false;
    bool m_dialogAccepted = false;
};

#endif // ADD_ACTIVITY_INFO_DIALOG_H
