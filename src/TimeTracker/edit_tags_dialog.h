#ifndef EDIT_TAGS_DIALOG_H
#define EDIT_TAGS_DIALOG_H

#include "tag_list_model.h"
#include "application_state.h"

#include <QDialog>
#include <QItemSelection>

namespace Ui {
class EditTagsDialog;
}

class EditTagsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditTagsDialog(QWidget *parent = 0);
    ~EditTagsDialog();

private slots:
    void tagListViewSelectionChanged(const QItemSelection& index);
    void tagColorPickButtonClicked(bool checked = false);

private:
    Ui::EditTagsDialog *ui;
    TagListModel* m_tagListModel;

    Tag* m_editTag = nullptr;
};

#endif // EDIT_TAGS_DIALOG_H
