#ifndef EDIT_SESSION_DIALOG_H
#define EDIT_SESSION_DIALOG_H

#include "core_types.h"

#include <QDialog>

namespace Ui {
class EditSessionDialog;
}

class EditSessionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditSessionDialog(QWidget *parent = 0);
    ~EditSessionDialog();

    void setSession(Session* session);
    Session* session() const;

    QString sessionName();
    QString sessionNote();
    QString sessionTags();
private:
    void populateFieldsFromSessionData();

    Ui::EditSessionDialog *ui = nullptr;

    Session* m_session = nullptr;
};

#endif // EDIT_SESSION_DIALOG_H
