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

bool DatabaseManager::establishDatabaseConnection(QString databasePath, QString* error)
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

bool DatabaseManager::closeDatabaseConnection()
{
    if (!m_database.isValid())
    {
        return false;
    }

    if (m_database.isOpen())
    {
        m_database.close();
    }

    return true;
}

bool DatabaseManager::loadActivityInfos()
{
    if (m_activityInfosLoaded)
        return true;

    QSqlQuery query = QSqlQuery(m_database);
    query.prepare("SELECT * FROM activity_info");

    if (!query.exec())
    {
        return false;
    }

    while (query.next())
    {
        ActivityInfo* info = g_app.m_activityInfoAllocator.allocate();
        ERR_VERIFY_CONTINUE(info);

        info->id = query.value("id").value<qint64>();
        info->name = query.value("name").value<QString>();
        info->color = query.value("color").value<qint64>();
        info->fieldNames = dbStringToStringList(query.value("field_names").value<QString>());
        //info->fieldTypes = //;dbStringToStringList(query.value.value<QString>());
        info->displayFormat = query.value("display_format").value<QString>();
        info->displayRules = query.value("display_rules").value<QString>();

        info->updateFormatter();

        m_activityInfos.insert(info->id, info);
        m_activityInfoList.append(info);
    }

    qSort(m_activityInfoList.begin(), m_activityInfoList.end(), activityInfoLessThanByName);
    m_activityInfosLoaded = true;
    return true;
}

QList<ActivityInfo*> DatabaseManager::activityInfos() const
{
    ERR_VERIFY_V(m_activityInfosLoaded, QList<ActivityInfo*>());
    return m_activityInfoList;
}

Activity* DatabaseManager::loadActivity(qint64 id) {
    ERR_VERIFY_V(id > 0, nullptr);

    Activity* activity = m_activities.value(id);
    if (activity != nullptr)
        return activity;

    QSqlQuery query = QSqlQuery(m_database);
    query.prepare("SELECT 1 FROM activity WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        return nullptr;
    }

    if (query.next()) {
        activity = g_app.m_activityAllocator.allocate();
        ERR_VERIFY_V(activity, nullptr);

        copyActivityValuesFromQuery(activity, &query);

        if (!m_activityInfosLoaded) {
            loadActivityInfos();
        }

        qint64 infoId = query.value("activity_info_id").value<qint64>();
        activity->info = m_activityInfos.value(infoId);

        m_activities.insert(id, activity);
    }

    return activity;
}

bool DatabaseManager::loadActivitiesAssociatedWithActivityInfo(ActivityInfo* info, QVector<Activity*>* associatedActivities)
{
    ERR_VERIFY_NULL_V(info, false);
    ERR_VERIFY_V(info->id > 0, false);
    ERR_VERIFY_NULL_V(associatedActivities, false);

    QSqlQuery query = QSqlQuery(m_database);
    query.prepare("SELECT * FROM activity WHERE activity_info_id = :activity_info_id");
    query.bindValue(":activity_info_id", info->id);

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

            activity->info = info;
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
    ERR_VERIFY_NULL_V(activity->info, false);
    ERR_VERIFY_V(activity->startTime <= activity->endTime, false);

    QSqlQuery query = QSqlQuery(m_database);

    bool isInsertAction = activity->id <= 0;

    if (!isInsertAction) {
        // Update activity.
        query.prepare("UPDATE activity SET activity_info_id = :activity_info_id, field_values = :field_values, start_time = :start_time, end_time = :end_time, intervals = :intervals WHERE id = :id");
        query.bindValue(":id", activity->id);
    } else {
        // Insert activity.
        query.prepare("INSERT INTO activity(activity_info_id, field_values, start_time, end_time, intervals) VALUES(:activity_info_id, :field_values, :start_time, :end_time, :intervals)");
    }

    query.bindValue(":activity_info_id", activity->info->id);
    query.bindValue(":start_time", activity->startTime);
    query.bindValue(":end_time", activity->endTime);
    query.bindValue(":field_values", dbStringListToString(activity->fieldValues));

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

bool DatabaseManager::saveActivityInfo(ActivityInfo* info)
{
    ERR_VERIFY_NULL_V(info, false);

    QSqlQuery query = QSqlQuery(m_database);

    bool isInsertAction = info->id <= 0;

    if (!isInsertAction)
    {
        // Update activity info.
        query.prepare("UPDATE activity_info SET name = :name, color = :color, field_names = :field_names, field_types = :field_types, display_format = :display_format, display_rules = :display_rules WHERE id = :id");
        query.bindValue(":id", info->id);
    }
    else
    {
        // Insert activity info.
        query.prepare("INSERT INTO activity_info(name, color, field_names, field_types, display_format, display_rules) VALUES(:name, :color, :field_names, :field_types, :display_format, :display_rules)");
    }

    query.bindValue(":name", info->name);
    query.bindValue(":field_names", dbStringListToString(info->fieldNames));
    query.bindValue(":color", info->color);
    query.bindValue(":field_types", dbActivityInfoFieldTypeVectorToString(info->fieldTypes));
    query.bindValue(":display_format", info->displayFormat);
    query.bindValue(":display_rules", info->displayRules);

    if (!query.exec())
    {
        return false;
    }

    if (isInsertAction)
    {
        info->id = query.lastInsertId().value<qint64>();
        ERR_VERIFY_V(info->id > 0, false);

        m_activityInfos.insert(info->id, info);
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

            qint64 activityInfoId = query.value("activity_info_id").value<qint64>();
            ActivityInfo* info = m_activityInfos.value(activityInfoId);
            if (!info)
            {
                if (!loadActivityInfos())
                {
                    return false;
                }
                info = m_activityInfos.value(activityInfoId);
                if (!info)
                {
                    return false;
                }
            }
            activity->info = info;
            copyActivityValuesFromQuery(activity, &query);
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
    activity->fieldValues = dbStringToStringList(query->value("field_values").value<QString>());
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
        "  field_values TEXT NOT NULL,\n"
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
