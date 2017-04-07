#include "edit_session_dialog.h"
#include "ui_edit_session_dialog.h"

EditSessionDialog::EditSessionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditSessionDialog)
{
    ui->setupUi(this);
}

void EditSessionDialog::setSession(Session* session)
{
    Q_ASSERT(session);
    Q_ASSERT(m_session == nullptr); // Session change not supported yet.
    if (m_session) return;

    m_session = session;
    populateFieldsFromSessionData();
}

QString EditSessionDialog::sessionName()
{
    Q_ASSERT(m_session);
    return ui->nameLineEdit->text();
}

QString EditSessionDialog::sessionNote()
{
    Q_ASSERT(m_session);
    return ui->noteLineEdit->text();
}

QString EditSessionDialog::sessionTags()
{
    Q_ASSERT(m_session);
    return ui->tagsLineEdit->text();
}

void EditSessionDialog::populateFieldsFromSessionData()
{
    Q_ASSERT(m_session);
    ui->nameLineEdit->setText(m_session->name);
    ui->noteLineEdit->setText(m_session->note);
}

Session* EditSessionDialog::session() const
{
    return m_session;
}

EditSessionDialog::~EditSessionDialog()
{
    delete ui;
}
