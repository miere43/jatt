#ifndef EDIT_TAGS_DIALOG_H
#define EDIT_TAGS_DIALOG_H

#include <QDialog>

namespace Ui {
class EditTagsDialog;
}

class EditTagsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditTagsDialog(QWidget *parent = 0);
    ~EditTagsDialog();

private:
    Ui::EditTagsDialog *ui;
};

#endif // EDIT_TAGS_DIALOG_H
