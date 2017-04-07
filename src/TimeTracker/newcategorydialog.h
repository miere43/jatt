#ifndef NEWCATEGORYDIALOG_H
#define NEWCATEGORYDIALOG_H

#include <QDialog>

namespace Ui {
class NewCategoryDialog;
}

class NewCategoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewCategoryDialog(QWidget *parent = 0);
    ~NewCategoryDialog();

private:
    Ui::NewCategoryDialog *ui;
};

#endif // NEWCATEGORYDIALOG_H
