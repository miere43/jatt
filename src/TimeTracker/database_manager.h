#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

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
    bool establishDatabaseConnection(QString databasePath, QString* error);
    bool closeDatabaseConnection();

    inline bool transaction() { return m_database.transaction(); }
    inline bool rollback() { return m_database.rollback(); }
    inline bool commit() { return m_database.commit(); }

    bool checkTables();
    bool createTables(QString* error);

    Activity* loadActivity(qint64 id);

    bool loadActivityInfos();
    QList<ActivityInfo*> activityInfos() const;

    bool loadActivitiesAssociatedWithActivityInfo(ActivityInfo* info, QVector<Activity*>* associatedActivities);
    bool saveActivity(Activity* activity);
    bool saveActivityInfo(ActivityInfo* activityInfo);

    bool deleteActivity(qint64 activityId);

    bool loadActivitiesBetweenStartAndEndTime(QVector<Activity*>* activities, qint64 startTime, qint64 endTime, bool checkIntervals = false);
    bool m_activityInfosLoaded = false;

    static void copyActivityValuesFromQuery(Activity* activity, QSqlQuery* query);

    QSqlDatabase m_database;
    QHash<qint64, ActivityInfo*> m_activityInfos;
    QHash<qint64, Activity*> m_activities;
    QList<ActivityInfo*> m_activityInfoList;
};

#endif // DATABASE_MANAGER_H
