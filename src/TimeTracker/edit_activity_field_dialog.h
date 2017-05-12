#ifndef EDIT_ACTIVITY_FIELD_DIALOG_H
#define EDIT_ACTIVITY_FIELD_DIALOG_H

#include <QDialog>
#include <QVariant>
#include "core_types.h"

namespace Ui {
class EditActivityFieldDialog;
}

class EditActivityFieldDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditActivityFieldDialog(Activity* activity, int fieldIndex, QWidget *parent = 0);
    ~EditActivityFieldDialog();

    QVariant newValue() const;
    inline Activity* activity() const { return m_activity; }
    inline int fieldIndex() const { return m_fieldIndex; }
private:
    Ui::EditActivityFieldDialog *ui;

    void setupUi();

    Activity* m_activity;
    int m_fieldIndex;
};

#endif // EDIT_ACTIVITY_FIELD_DIALOG_H
