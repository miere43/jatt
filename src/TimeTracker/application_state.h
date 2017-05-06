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
#include "plugin_manager.h"

class MainWindow;

class ApplicationState : public QObject
{
    Q_OBJECT

private:
    DatabaseManager m_databaseManager;
    MainWindow* m_mainWindow = nullptr;

    int m_offsetFromUtc;
    int m_currentDaySinceEpochUtc;

    bool m_initialized = false;
public:
    PluginManager m_pluginManager;
    QString appTitle;

    inline int offsetFromUtc() { return m_offsetFromUtc; }
    inline int currentDaySinceEpochUtc() { return m_currentDaySinceEpochUtc; }

    explicit ApplicationState(QObject *parent = 0);
    bool initialize(QString* errorMessage);

    DatabaseManager* database();
    MainWindow* mainWindow();

    void setMainWindow(MainWindow* mainWindow);

    BlockAllocator<ActivityInfo> m_activityInfoAllocator;
    BlockAllocator<Activity> m_activityAllocator;
private slots:
    void appAboutToQuit();
private:

};

extern ApplicationState g_app;

#endif // APPLICATION_STATE_H
