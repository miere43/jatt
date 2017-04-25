#ifndef ADD_ACTIVITY_DIALOG_H
#define ADD_ACTIVITY_DIALOG_H

#include "core_types.h"

#include <QDialog>

namespace Ui {
class AddActivityDialog;
}

class AddActivityDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddActivityDialog(QWidget *parent = 0);
    ~AddActivityDialog();

    Activity* constructActivity();

private slots:
    void on_addFieldButton_clicked();
    void addFieldDialogFieldAdded();
    void addFieldDialogFinished(int result);

    void on_addActivityInfo_clicked();
    void addActivityInfoDialogInfoAdded();
    void addActivityInfoDialogFinished(int result);

    void on_activityInfoComboBox_currentIndexChanged(int index);
    void on_AddActivityDialog_finished(int result);
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
private:
    Ui::AddActivityDialog *ui;
    ActivityInfo* m_currentActivityInfo;
    QList<QWidget*> m_activityInfoWidgets;
    QList<QWidget*> m_activityInfoFieldWidgets;

    bool m_activityWasConstructed = false;
    void setActivityInfo(ActivityInfo* info);
};

#endif // ADD_ACTIVITY_DIALOG_H
