#ifndef ADD_ACTIVITY_DIALOG_H
#define ADD_ACTIVITY_DIALOG_H

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

private:
    Ui::AddActivityDialog *ui;
};

#endif // ADD_ACTIVITY_DIALOG_H
