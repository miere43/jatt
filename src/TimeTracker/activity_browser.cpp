#include "activity_browser.h"
#include "ui_activity_browser.h"

#include "application_state.h"
#include "core_types.h"
#include "search_query.h"

#include <QMessageBox>

ActivityBrowser::ActivityBrowser(QWidget * parent) :
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
    DatabaseManager * db = g_app.database();
    m_activities.clear();
    m_activityTableModel->setActivities(m_activities);

    SearchQuery searchQuery = SearchQuery(query, g_app.database()->activityCategories().toVector());
    if (!searchQuery.isValid())
    {
        QMessageBox::critical(this, "Error", "Search query is not valid.");
        return;
    }

    SearchQuery::GeneratedSqlQuery queryInfo = searchQuery.sqlQuery();
    ERR_VERIFY(queryInfo.isValid);

    if (!db->executeSearchQuery(&m_activities, &queryInfo))
    {
        QMessageBox::critical(this, "Error", "Unable to execute your query.");
        return;
    }

    m_activityTableModel->setActivities(m_activities);
}


void ActivityBrowser::executeSearchAction()
{
    QString query = ui->searchQueryLineEdit->text();

    this->search(query);
}

void ActivityBrowser::on_searchResultsTable_activated(const QModelIndex& index)
{
    QVariant data = index.data(Qt::UserRole);
    void* value = data.value<void*>();

    auto activity = reinterpret_cast<Activity*>(value);
    if (!activity)
    {
        return;
    }

    auto main_window = qobject_cast<MainWindow*>(parent());
    ERR_VERIFY(main_window);

    const qint64 day = (activity->startTime + g_app.localOffsetFromUtc()) / 86400000LL;
    main_window->setViewDay(day);
    main_window->selectActivityById(activity->id);
}
