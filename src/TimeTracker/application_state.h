#ifndef APPLICATION_STATE_H
#define APPLICATION_STATE_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QString>
#include <QVector>
#include <QTimeZone>
#include <QDebug>

#include "core_types.h"
#include "block_allocator.h"
#include "database_manager.h"

class MainWindow;

class ApplicationState : public QObject
{
    Q_OBJECT

private:
    DatabaseManager m_databaseManager;
    MainWindow* m_mainWindow = nullptr;

    int m_localOffsetFromUtc;

    bool m_initialized = false;
public:
    QString appTitle;

    inline int localOffsetFromUtc() { return m_localOffsetFromUtc; }
    inline qint64 currentDaySinceEpochUtc() { return getCompleteDaysSinceEpoch(getCurrentDateTimeUtc()); }
    inline qint64 currentDaySinceEpochLocal() { return getCompleteDaysSinceEpoch(getCurrentDateTimeUtc() + localOffsetFromUtc()); }

    explicit ApplicationState(QObject *parent = nullptr);
    bool initialize(QString* errorMessage);

    DatabaseManager* database();
    MainWindow* mainWindow();

    void setMainWindow(MainWindow* mainWindow);

    BlockAllocator<ActivityCategory> m_activityCategoryAllocator;
    BlockAllocator<Activity> m_activityAllocator;
private slots:
//    void appAboutToQuit();
private:

};

extern ApplicationState g_app;

#endif // APPLICATION_STATE_H
