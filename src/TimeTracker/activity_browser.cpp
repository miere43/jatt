#include "activity_browser.h"
#include "ui_activity_browser.h"

#include "application_state.h"
#include "core_types.h"

#include <QMessageBox>

ActivityBrowser::ActivityBrowser(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ActivityBrowser)
{
    ui->setupUi(this);
    m_activityTableModel = new ActivityTableModel(this);
    ui->searchResultsTable->setModel(m_activityTableModel);
}

ActivityBrowser::~ActivityBrowser()
{
    delete ui;
}

void ActivityBrowser::search(QString query)
{
    if (query.isEmpty())
    {
        QMessageBox::information(this, QStringLiteral("Search query is empty"), "");
        return;
    }

    DatabaseManager * db = g_app.database();
    m_activities.clear();

    ERR_VERIFY(db->loadActivitiesBetweenStartAndEndTime(&m_activities, 0i64, INT64_MAX, false));

    m_activityTableModel->setActivities(m_activities);
}

void ActivityBrowser::on_searchButton_clicked()
{
    QString query = ui->searchQueryLineEdit->text();

    this->search(query);
}
