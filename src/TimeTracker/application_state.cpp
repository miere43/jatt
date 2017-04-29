#include "application_state.h"

#include <QDebug>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QVariant>
#include <QCoreApplication>
#include "common.h"
#include "date_time.h"
#include "mainwindow.h"

ApplicationState::ApplicationState(QObject *parent)
    : QObject(parent)
{

}

void ApplicationState::initialize()
{
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
            this, &ApplicationState::appAboutToQuit);

    m_properties.firstActivityDayUtc = -1;
    m_properties.localTimeZoneOffsetFromUtc = 0;

    Q_ASSERT(database()->loadProperties(&m_properties));
    qDebug() << "firstActivityDayUtc =" << m_properties.firstActivityDayUtc;

    if (m_properties.firstActivityDayUtc == -1)
    {
        m_properties.firstActivityDayUtc = getCompleteDaysSinceEpoch(getCurrentDateTimeUtc());
        qDebug() << "firstActivityDay property is missing, setting it to " << m_properties.firstActivityDayUtc;
        Q_ASSERT(database()->saveProperty("firstActivityDayUtc", QString::number(m_properties.firstActivityDayUtc)));
    }
    m_currentDaySinceEpochUtc = getCompleteDaysSinceEpoch(getCurrentDateTimeUtc());
}

bool ApplicationState::openDatabase()
{
    return m_databaseManager.establishDatabaseConnection();
}

DatabaseManager* ApplicationState::database()
{
    return &m_databaseManager;
}

MainWindow* ApplicationState::mainWindow() {
    return m_mainWindow;
}

void ApplicationState::setMainWindow(MainWindow* mainWindow) {
    m_mainWindow = mainWindow;
}

void ApplicationState::appAboutToQuit()
{

}

ApplicationState g_app;
