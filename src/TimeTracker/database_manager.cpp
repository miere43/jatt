#include "database_manager.h"

#include <QtSql/QSqlError>
#include <QVariant>
#include <QByteArray>

#include "utilities.h"
#include "common.h"
#include "application_state.h"

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{

}

bool DatabaseManager::establishDatabaseConnection()
{
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    if (!m_database.isValid())
    {
        qCritical() << "Unable to add SQLite database: " << m_database.lastError().text();
        return false;
    }

    const char* databaseName =
#ifdef APP_DEBUG_DB
            "D:/test.s3db"; // that's misleading name for a macro :/
#else
            "D:/test_debug.s3db";
#endif
    qDebug() << databaseName;
    m_database.setConnectOptions(QString("QSQLITE_OPEN_URI=") + databaseName);
    m_database.setDatabaseName(databaseName); // @TODO temporary
    if (!m_database.open()) // @TODO: this is always true because it will create database if one doesn't exist.
    {
        qCritical() << "Unable to open SQLite database: " << m_database.lastError().text();
        // QSqlDatabase::removeDatabase @TODO is this required?
        return false;
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

bool DatabaseManager::loadProperties(UserProperties* properties)
{
    Q_ASSERT(properties);

    QSqlQuery query = QSqlQuery(m_database);
    query.setForwardOnly(true);
    if (!query.exec("SELECT * FROM user_property"))
    {
        APP_ERRSTREAM << "Query failed." << query.lastError().text();
        return false;
    }

    while (query.next())
    {
        QString property = query.value("property").toString();
        QVariant value = query.value("data");
        if (property == QLatin1Literal("firstActivityDayUtc"))
        {
            properties->firstActivityDayUtc = value.value<qint64>();
        }
        if (property == QLatin1Literal("localTimeZoneOffsetFromUtc"))
        {
            properties->localTimeZoneOffsetFromUtc = value.value<qint64>();
        }
        else
        {
            QString stringValue = value.value<QString>();
            properties->customProperties.insert(stringValue, stringValue);
        }
    }

    return true;
}

bool DatabaseManager::saveProperty(QString property, QString value)
{
    Q_ASSERT(!property.isNull() && !property.isEmpty());

    QSqlQuery query = QSqlQuery(m_database);
    query.prepare("INSERT INTO user_property(property, data) VALUES(:property, :data)");
    query.bindValue(":property", property);
    query.bindValue(":data", value);

    if (!query.exec())
    {
        APP_ERRSTREAM << "Query failed." << query.lastError().text();
        return false;
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
        APP_ERRSTREAM << "Query failed." << query.lastError().text();
        return false;
    }


    while (query.next())
    {
        ActivityInfo* info = g_app.m_activityInfoAllocator.allocate();
        Q_ASSERT(info);

        info->id = query.value("id").value<qint64>();
        info->name = query.value("name").value<QString>();
        info->color = query.value("color").value<qint64>();
        info->fieldNames = dbStringToStringList(query.value("field_names").value<QString>());
        //info->fieldTypes = //;dbStringToStringList(query.value.value<QString>());
        info->displayFormat = query.value("display_format").value<QString>();
        info->displayRules = query.value("display_rules").value<QString>();

        info->updateFormatter();

        m_activityInfos.insert(info->id, info);
    }

    qDebug() << "loaded " << query.size() << "act infos";

    m_activityInfosLoaded = true;
    return true;
}

QList<ActivityInfo*> DatabaseManager::activityInfos() const
{
    Q_ASSERT(m_activityInfosLoaded);
    return m_activityInfos.values();
}

bool DatabaseManager::loadActivitiesAssociatedWithActivityInfo(ActivityInfo* info, QVector<Activity*>* associatedActivities)
{
    Q_ASSERT(info);
    Q_ASSERT(info->id > 0);
    Q_ASSERT(associatedActivities);

    QSqlQuery query = QSqlQuery(m_database);
    query.prepare("SELECT * FROM activity WHERE activity_info_id = :activity_info_id");
    query.bindValue(":activity_info_id", info->id);

    if (!query.exec())
    {
        APP_ERRSTREAM << "Query failed." << query.lastError().text();
        return false;
    }

    while (query.next())
    {
        qint64 activityId = query.value("id").value<qint64>();
        Activity* activity = m_activities.value(activityId);
        if (!activity)
        {
            activity = g_app.m_activityAllocator.allocate();
            if (!activity)
                return false;

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
    Q_ASSERT(activity);
    Q_ASSERT(activity->info);
    Q_ASSERT(activity->startTime <= activity->endTime);

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
        APP_ERRSTREAM << "Query failed." << query.lastError().text();
        return false;
    }

    if (isInsertAction)
    {
        activity->id = query.lastInsertId().value<qint64>();
        Q_ASSERT(activity->id > 0);
        m_activities.insert(activity->id, activity);
    }

    return true;
}

bool DatabaseManager::deleteActivity(qint64 activityId) {
    Q_ASSERT(activityId > 0);

    QSqlQuery query = QSqlQuery(m_database);
    query.prepare("DELETE FROM activity WHERE id = :id");
    query.bindValue(":id", activityId);

    if (!query.exec()) {
        APP_ERRSTREAM << "unable to delete:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::saveActivityInfo(ActivityInfo* info)
{
    Q_ASSERT(info);

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
        APP_ERRSTREAM << "Query failed." << query.lastError().text();
        return false;
    }

    if (isInsertAction)
    {
        info->id = query.lastInsertId().value<qint64>();
        Q_ASSERT(info->id > 0);
        m_activityInfos.insert(info->id, info);
    }

    return true;
}

bool DatabaseManager::loadActivitiesBetweenStartAndEndTime(QVector<Activity*>* activities, qint64 startTime, qint64 endTime, bool checkIntervals)
{
    Q_ASSERT(activities);
    Q_ASSERT(startTime >= 0);
    Q_ASSERT(endTime >= 0);
    Q_ASSERT(startTime < endTime);

    QSqlQuery query = QSqlQuery(m_database);
    // oh god forgive me for this query, this is the best I can do.
    query.prepare("SELECT * FROM activity WHERE (:start_time >= start_time AND :end_time <= end_time) OR (:start_time >= start_time AND :end_time >= start_time AND end_time >= :start_time) OR (:start_time <= start_time AND :end_time >= start_time)");
    query.bindValue(":start_time", startTime);
    query.bindValue(":end_time", endTime);

    if (!query.exec())
    {
        APP_ERRSTREAM << "Query failed." << query.lastError().text();
        return false;
    }

    qDebug() << "Activities between" << startTime / (double)86400000 << "-" << endTime / (double)86400000 << "are:";

    while (query.next())
    {
        qint64 activityId = query.value("id").value<qint64>();
        Activity* activity = m_activities.value(activityId);
        if (!activity)
        {
            activity = g_app.m_activityAllocator.allocate();
            Q_ASSERT(activity);
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

        // qDebug() << "-" << activity->id << activity->displayString() << "|" << activity->startTime / (double)86400000 << activity->endTime / (double)86400000;

        if (!checkIntervals || (checkIntervals && activity->belongsToTimePeriod(startTime, endTime))) {
           activities->append(activity);
        }
    }

    return true;
}

void DatabaseManager::copyActivityValuesFromQuery(Activity *activity, QSqlQuery *query)
{
    Q_ASSERT(activity);
    Q_ASSERT(query);

    activity->id = query->value("id").value<qint64>();
    activity->fieldValues = dbStringToStringList(query->value("field_values").value<QString>());
    activity->startTime = query->value("start_time").value<qint64>();
    activity->endTime = query->value("end_time").value<qint64>();

    QByteArray binaryIntervals = query->value("intervals").toByteArray();
    Q_ASSERT(binaryIntervals.count() % sizeof(Interval) == 0);

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
