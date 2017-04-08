#ifndef ADD_FIELD_DIALOG_H
#define ADD_FIELD_DIALOG_H

#include <QDialog>
#include "core_types.h"

namespace Ui {
class AddFieldDialog;
}

class AddFieldDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFieldDialog(ActivityInfo* info, QWidget *parent = 0);
    ~AddFieldDialog();
private slots:
    void on_buttonBox_accepted();
signals:
    void fieldAdded();
private:
    Ui::AddFieldDialog *ui;
    ActivityInfo* m_activityInfo;
};

#endif // ADD_FIELD_DIALOG_H
