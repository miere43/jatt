#include "database_manager.h"
#include "application_state.h"
#include "error_macros.h"
#include "utilities.h"

#include <QtSql/QSqlError>
#include <QByteArray>
#include <QVariant>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{

}

bool DatabaseManager::connectToDatabase(QString databasePath, QString* error)
{
    ERR_VERIFY_NULL_V(error, false);

    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        *error = "SQLite driver is not available.";
        return false;
    }

    m_database = QSqlDatabase::addDatabase("QSQLITE");
    if (!m_database.isValid())
    {
        *error = "Unable to add SQLite database, error: " + m_database.lastError().text();
        return false;
    }

    m_database.setDatabaseName(databasePath);
    if (!m_database.open() || !m_database.isOpen()) // @TODO: this is always true because it will create database if one doesn't exist.
    {
        // QSqlDatabase::removeDatabase @TODO is this required?
        *error = "Unable to open SQLite database: " + m_database.lastError().text();
        return false;
    }

    if (!checkTables()) {
        QString createError;
        if (!createTables(&createError)) {
            *error = "Unable to create required tables: " + createError;
            return false;
        }
    }

    return true;
}

bool DatabaseManager::closeDatabase()
{
    if (!m_database.isValid()) {
        return false;
    }

    if (m_database.isOpen()) {
        m_database.close();
    }

    return true;
}

bool DatabaseManager::loadActivityCategories()
{
    if (m_activityCategoriesLoaded)
        return true;

    QSqlQuery query = QSqlQuery(m_database);
    query.prepare("SELECT * FROM activity_info");

    if (!query.exec())
    {
        return false;
    }

    while (query.next())
    {
        ActivityCategory* category = g_app.m_activityCategoryAllocator.allocate();
        ERR_VERIFY_CONTINUE(category);

        category->id    = query.value("id").value<qint64>();
        category->name  = query.value("name").value<QString>();
        category->color = query.value("color").value<qint64>();

        m_activityCategories.insert(category->id, category);
        m_activityCategoriesList.append(category);
    }

    qSort(m_activityCategoriesList.begin(), m_activityCategoriesList.end(), activityCategoryLessThanByName);
    m_activityCategoriesLoaded = true;
    return true;
}

QList<ActivityCategory*> DatabaseManager::activityCategories() const
{
    ERR_VERIFY_V(m_activityCategoriesLoaded, QList<ActivityCategory*>());
    return m_activityCategoriesList;
}

Activity* DatabaseManager::loadActivity(qint64 id)
{
    ERR_VERIFY_V(id > 0, nullptr);

    Activity* activity = m_activities.value(id);
    if (activity != nullptr)
        return activity;

    QSqlQuery query = QSqlQuery(m_database);
    query.prepare("SELECT * FROM activity WHERE id = :id LIMIT 1");
    query.bindValue(":id", id);

    if (!query.exec()) {
        return nullptr;
    }

    if (query.next()) {
        activity = g_app.m_activityAllocator.allocate();
        ERR_VERIFY_V(activity, nullptr);

        copyActivityValuesFromQuery(activity, &query);

        if (!m_activityCategoriesLoaded) {
            loadActivityCategories();
        }

        qint64 categoryId = query.value("activity_info_id").value<qint64>();
        activity->category = m_activityCategories.value(categoryId);

        m_activities.insert(id, activity);
    }

    return activity;
}

bool DatabaseManager::loadActivitiesAssociatedWithActivityCategory(ActivityCategory* category, QVector<Activity*>* associatedActivities)
{
    ERR_VERIFY_NULL_V(category, false);
    ERR_VERIFY_V(category->id > 0, false);
    ERR_VERIFY_NULL_V(associatedActivities, false);

    QSqlQuery query = QSqlQuery(m_database);
    query.prepare("SELECT * FROM activity WHERE activity_info_id = :activity_info_id");
    query.bindValue(":activity_info_id", category->id);

    if (!query.exec())
    {
        return false;
    }

    while (query.next())
    {
        qint64 activityId = query.value("id").value<qint64>();
        Activity* activity = m_activities.value(activityId);
        if (!activity)
        {
            activity = g_app.m_activityAllocator.allocate();
            ERR_VERIFY_NULL_V(activity, false);

            activity->category = category;
            copyActivityValuesFromQuery(activity, &query);
            m_activities.insert(activity->id, activity);
        }

        associatedActivities->append(activity);
    }

    return true;
}

bool DatabaseManager::saveActivity(Activity* activity)
{
    ERR_VERIFY_NULL_V(activity, false);
    ERR_VERIFY_NULL_V(activity->category, false);
    ERR_VERIFY_V(activity->startTime <= activity->endTime, false);

    QSqlQuery query = QSqlQuery(m_database);

    bool isInsertAction = activity->id <= 0;

    if (!isInsertAction) {
        // Update activity.
        query.prepare("UPDATE activity SET activity_info_id = :activity_info_id, name = :name, note = :note, start_time = :start_time, end_time = :end_time, intervals = :intervals WHERE id = :id");
        query.bindValue(":id", activity->id);
    } else {
        // Insert activity.
        query.prepare("INSERT INTO activity(activity_info_id, name, note, start_time, end_time, intervals) VALUES(:activity_info_id, :name, :note, :start_time, :end_time, :intervals)");
    }

    query.bindValue(":activity_info_id", activity->category->id);
    query.bindValue(":start_time", activity->startTime);
    query.bindValue(":end_time", activity->endTime);
    query.bindValue(":name", activity->name);
    query.bindValue(":note", activity->note);

    QByteArray intervals((const char*)activity->intervals.data(), sizeof(Interval) * activity->intervals.count());
    query.bindValue(":intervals", intervals);

    if (!query.exec())
    {
        return false;
    }

    if (isInsertAction)
    {
        activity->id = query.lastInsertId().value<qint64>();
        ERR_VERIFY_V(activity->id > 0, false);

        m_activities.insert(activity->id, activity);
    }

    return true;
}

bool DatabaseManager::deleteActivity(qint64 activityId) {
    ERR_VERIFY_V(activityId > 0, false);

    QSqlQuery query = QSqlQuery(m_database);
    query.prepare("DELETE FROM activity WHERE id = :id");
    query.bindValue(":id", activityId);

    if (!query.exec()) {
        return false;
    }

    return true;
}

bool DatabaseManager::saveActivityCategory(ActivityCategory * category)
{
    ERR_VERIFY_NULL_V(category, false);

    QSqlQuery query = QSqlQuery(m_database);

    bool isInsertAction = category->id <= 0;

    if (!isInsertAction)
    {
        // Update activity category.
        query.prepare("UPDATE activity_info SET name = :name, color = :color WHERE id = :id");
        query.bindValue(":id", category->id);
    }
    else
    {
        // Insert activity category.
        query.prepare("INSERT INTO activity_info(name, color) VALUES(:name, :color)");
    }

    query.bindValue(":name", category->name);
    query.bindValue(":color", category->color);

    if (!query.exec())
    {
        return false;
    }

    if (isInsertAction)
    {
        category->id = query.lastInsertId().value<qint64>();
        ERR_VERIFY_V(category->id > 0, false);

        m_activityCategories.insert(category->id, category);
    }

    return true;
}

bool DatabaseManager::loadActivitiesBetweenStartAndEndTime(QVector<Activity*>* activities, qint64 startTime, qint64 endTime, bool checkIntervals)
{
    ERR_VERIFY_NULL_V(activities, false);
    ERR_VERIFY_V(startTime >= 0, false);
    ERR_VERIFY_V(endTime >= 0, false);
    ERR_VERIFY_V(startTime < endTime, false);

    QSqlQuery query = QSqlQuery(m_database);
    // oh god forgive me for this query, this is the best I can do.
    query.prepare("SELECT * FROM activity WHERE (:start_time >= start_time AND :end_time <= end_time) OR (:start_time >= start_time AND :end_time >= start_time AND end_time >= :start_time) OR (:start_time <= start_time AND :end_time >= start_time)");
    query.bindValue(":start_time", startTime);
    query.bindValue(":end_time", endTime);

    if (!query.exec())
    {
        return false;
    }

    while (query.next())
    {
        qint64 activityId = query.value("id").value<qint64>();
        Activity* activity = m_activities.value(activityId);
        if (!activity)
        {
            activity = g_app.m_activityAllocator.allocate();
            ERR_VERIFY_CONTINUE(activity);

            copyActivityValuesFromQuery(activity, &query);
            loadAssociatedActivityCategory(activity, &query);

            m_activities.insert(activityId, activity);
        }

        if (!checkIntervals || (checkIntervals && activity->belongsToTimePeriod(startTime, endTime))) {
           activities->append(activity);
        }
    }

    return true;
}

void DatabaseManager::copyActivityValuesFromQuery(Activity *activity, QSqlQuery *query)
{
    ERR_VERIFY_NULL(activity);
    ERR_VERIFY_NULL(query);

    activity->id = query->value("id").value<qint64>();
    activity->name = query->value("name").value<QString>();
    activity->note = query->value("note").value<QString>();
    activity->startTime = query->value("start_time").value<qint64>();
    activity->endTime = query->value("end_time").value<qint64>();

    QByteArray binaryIntervals = query->value("intervals").toByteArray();
    ERR_VERIFY(binaryIntervals.count() % sizeof(Interval) == 0);

    int numIntervals = binaryIntervals.count() / sizeof(Interval);
    qint64* data = (qint64*)binaryIntervals.data();
    binaryIntervals.reserve(numIntervals);
    for (int i = 0; i < numIntervals; ++i)
    {
        activity->intervals.append(Interval {
                                       *(data + (i * 2)),
                                       *(data + (i * 2) + 1)
                                    });
    }

    binaryIntervals.clear();
}

bool DatabaseManager::checkTables() {
    static const char* tableNames[] = {
        "activity",
        "activity_info",
    };

    QSqlQuery query(m_database);
    query.prepare("SELECT 1 FROM sqlite_master WHERE type = ? AND name = ?");
    query.bindValue(0, "table");

    const int numTables = sizeof(tableNames) / sizeof(tableNames[0]);
    for (int i = 0; i < numTables; ++i) {
        query.bindValue(1, tableNames[i]);

        if (!query.exec()) {
            continue;
        }

        if (query.next() == false) {
            return false;
        }
        query.finish();
    }

    return true;
}

bool DatabaseManager::createTables(QString* error) {
    ERR_VERIFY_NULL_V(error, false);

    // Only one statement per query :(
    static const char* queries[] = {
        "CREATE TABLE activity_info(\n"
        "  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,\n"
        "  name TEXT NOT NULL,\n"
        "  color INTEGER,\n"
        "  field_names TEXT,\n"
        "  field_types TEXT,\n"
        "  display_format TEXT,\n"
        "  display_rules TEXT\n"
        ");\n",

        "CREATE TABLE activity(\n"
        "  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,\n"
        "  activity_info_id INTEGER NOT NULL,\n"
        "  name TEXT NOT NULL,\n"
        "  note TEXT,\n"
        "  start_time INTEGER NOT NULL,\n"
        "  end_time INTEGER NOT NULL,\n"
        "  intervals BLOB,\n"
        "  FOREIGN KEY(activity_info_id) REFERENCES activity_info(id)\n"
        ");\n",
    };

    int numQueries = sizeof(queries) / sizeof(queries[0]);
    QSqlQuery query(m_database);
    for (int i = 0; i < numQueries; ++i) {
        if (!query.exec(queries[i])) {
            *error = query.lastError().text();
            return false;
        }
        query.finish();
    }

    return true;
}


bool DatabaseManager::executeSearchQuery(QVector<Activity *> * activities, SearchQuery::GeneratedSqlQuery * searchQuery)
{
    ERR_VERIFY_NULL_V(activities, false);
    ERR_VERIFY_NULL_V(searchQuery, false);

    QSqlQuery query = QSqlQuery(m_database);
    query.prepare(searchQuery->query);
    for (const QVariant& arg : searchQuery->args)
    {
        query.addBindValue(arg);
    }

    if (!query.exec())
    {
        return false;
    }
    else
    {
        while (query.next())
        {
            qint64 activityId = query.value("id").value<qint64>();
            Activity * activity = m_activities.value(activityId);
            if (!activity)
            {
                activity = g_app.m_activityAllocator.allocate();
                ERR_VERIFY_CONTINUE(activity);

                copyActivityValuesFromQuery(activity, &query);
                loadAssociatedActivityCategory(activity, &query);

                m_activities.insert(activityId, activity);
            }

            activities->append(activity);
        }
    }

    return true;
}

void DatabaseManager::loadAssociatedActivityCategory(Activity * activity, QSqlQuery * query)
{
    qint64 activityCategoryId = query->value("activity_info_id").value<qint64>();
    ActivityCategory* category = m_activityCategories.value(activityCategoryId);
    if (!category)
    {
        if (!loadActivityCategories())
        {
            qDebug() << __FUNCTION__ << "unable to load activity categories.";
            return;
        }
        category = m_activityCategories.value(activityCategoryId);
        if (!category)
        {
            qDebug() << __FUNCTION__ << "activity info with id" << activityCategoryId << "is missing.";
            return;
        }
    }
    activity->category = category;
}
