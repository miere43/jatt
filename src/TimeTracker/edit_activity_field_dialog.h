#ifndef EDIT_ACTIVITY_FIELD_DIALOG_H
#define EDIT_ACTIVITY_FIELD_DIALOG_H

#include <QDialog>
#include "core_types.h"

namespace Ui {
class EditActivityFieldDialog;
}

class EditActivityFieldDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditActivityFieldDialog(Activity* activity, QWidget *parent = 0);
    ~EditActivityFieldDialog();

    QString newName() const;
    QString newNote() const;
    inline bool isNameFieldChanged() const { return m_isNameFieldChanged; }
    inline bool isNoteFieldChanged() const { return m_isNoteFieldChanged; }

    inline Activity* activity() const { return m_activity; }
private:
    Ui::EditActivityFieldDialog *ui;
    Activity* m_activity;

    bool m_isNameFieldChanged = false;
    bool m_isNoteFieldChanged = false;
};

#endif // EDIT_ACTIVITY_FIELD_DIALOG_H
