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
    explicit EditActivityFieldDialog(Activity * activity, QWidget * parent = nullptr);
    ~EditActivityFieldDialog();

    QString newName() const;
    QString newNote() const;
    ActivityCategory* newActivityCategory() const;
    bool newFavorite() const;
    bool isNameFieldChanged() const;
    bool isNoteFieldChanged() const;
    bool isActivityCategoryChanged() const;
    bool isFavoriteChanged() const;

    inline Activity * activity() const { return m_activity; }
private slots:
    void on_prevActivityLabel_linkActivated(const QString &link);

    void on_typeComboBox_currentIndexChanged(int index);

private:
    void updatePrevActivityName(ActivityCategory * category);

    Ui::EditActivityFieldDialog * ui;
    Activity * m_activity = nullptr;
    QString m_prevActivityName;
};

#endif // EDIT_ACTIVITY_FIELD_DIALOG_H
