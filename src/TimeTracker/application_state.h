#ifndef APPLICATION_STATE_H
#define APPLICATION_STATE_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QString>
#include <QLinkedList>
#include <QVector>
#include <QTimeZone>
#include "core_types.h"
#include "block_allocator.h"
#include "database_manager.h"

class ApplicationState : public QObject
{
    Q_OBJECT

private:
    DatabaseManager m_databaseManager;
public:
    UserProperties m_properties; // @TODO shouldn't be public.
    int m_currentDaySinceEpochUtc;

    explicit ApplicationState(QObject *parent = 0);

    // @TODO
    // bool initializeSubsystems();

    bool openDatabase(); // @Obsolete
    void initialize();

    DatabaseManager* database();

    BlockAllocator<ActivityInfo> m_activityInfoAllocator;
    BlockAllocator<Activity> m_activityAllocator;
private slots:
    void appAboutToQuit();
private:

};

extern ApplicationState g_app;

#endif // APPLICATION_STATE_H
