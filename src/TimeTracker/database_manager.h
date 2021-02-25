#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QHash>

#include "core_types.h"
#include "block_allocator.h"
#include "search_query.h"

class DatabaseManager : public QObject
{
public:
    explicit DatabaseManager(QObject * parent = nullptr);
    bool connectToDatabase(QString databasePath, QString * error);
    bool closeDatabase();

    inline bool transaction() { return m_database.transaction(); }
    inline bool rollback() { return m_database.rollback(); }
    inline bool commit() { return m_database.commit(); }

    bool checkTables();
    bool createTables(QString * error);

    Activity* loadActivity(qint64 id);

    bool loadActivityCategories();
    QList<ActivityCategory*> activityCategories() const;
    ActivityCategory* activityCategoryById(qint64 id);

    bool loadActivitiesAssociatedWithActivityCategory(ActivityCategory * category, QVector<Activity *> * associatedActivities);
    bool saveActivity(Activity* activity);
    bool saveActivityCategory(ActivityCategory * category);

    bool deleteActivity(qint64 activityId);

    bool loadActivitiesBetweenStartAndEndTime(QVector<Activity *> * activities, qint64 startTime, qint64 endTime, bool checkIntervals = false);

    bool executeSearchQuery(QVector<Activity *> * activities, SearchQuery::GeneratedSqlQuery * searchQuery);

    QString loadFirstActivityNameBeforeEndTime(ActivityCategory * category, qint64 timestamp);

    bool m_activityCategoriesLoaded = false;
    QSqlDatabase m_database;
    QHash<qint64, ActivityCategory *> m_activityCategories;
    QHash<qint64, Activity *> m_activities;
    QList<ActivityCategory *> m_activityCategoriesList;

private:
    static void copyActivityValuesFromQuery(Activity * activity, QSqlQuery * query);
    void loadAssociatedActivityCategory(Activity * activity, QSqlQuery * query);
};

#endif // DATABASE_MANAGER_H
