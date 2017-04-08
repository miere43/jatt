#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

// Debug variable, doesn't commit changes to database when set to 1.
// e.g. uploadSession(...) will not write anything to database, but method will return true.
#define APP_DBMANAGER_NO_COMMIT 0

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QHash>
#include "core_types.h"
#include "block_allocator.h"

class DatabaseManager : public QObject
{
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    bool establishDatabaseConnection();
    bool closeDatabaseConnection();

    bool loadProperties(UserProperties* properties);
    bool saveProperty(QString property, QString value);

    bool loadActivityInfos();
    QList<ActivityInfo*> activityInfos() const;

    bool loadActivitiesAssociatedWithActivityInfo(ActivityInfo* info, QVector<Activity*>* associatedActivities);
    bool saveActivity(Activity* activity);
    bool saveActivityInfo(ActivityInfo* activityInfo);

    bool loadActivitiesBetweenStartAndEndTime(QVector<Activity*>* activities, qint64 startTime, qint64 endTime);
    bool m_activityInfosLoaded = false;

    static void copyActivityValuesFromQuery(Activity* activity, QSqlQuery* query);

    QSqlDatabase m_database;
    QHash<qint64, ActivityInfo*> m_activityInfos;
    QHash<qint64, Activity*> m_activities;
};

#endif // DATABASE_MANAGER_H
