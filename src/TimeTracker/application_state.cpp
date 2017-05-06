#include "application_state.h"

#include <QDebug>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QVariant>
#include <QCoreApplication>
#include <QSettings>

#include "consts.h"
#include "common.h"
#include "date_time.h"
#include "mainwindow.h"

ApplicationState::ApplicationState(QObject *parent)
    : QObject(parent)
{
#ifdef APP_DEBUG_DB
    appTitle = QStringLiteral("Qt Time Tracker");
#else
    appTitle = QStringLiteral("(Debug) Qt Time Tracker");
#endif
}

bool ApplicationState::initialize(QString* error)
{
    Q_ASSERT(error);

    QSettings settings;
    settings.beginGroup("main");

    QString databasePath = settings.value("databasePath").value<QString>();
    if (databasePath.isEmpty() || !QFile::exists(databasePath)) {
        QString appDirPath = QCoreApplication::applicationDirPath();
        QDir appDir(appDirPath);
        appDir.mkdir("data");
        if (!appDir.cd("data")) {
            *error = QString("Unable to create directory 'data' in '%0'.").arg(appDirPath);
            return false;
        }
        databasePath = appDir.filePath("user.s3db");
        settings.setValue("databasePath", databasePath);
    }

    if (!m_databaseManager.establishDatabaseConnection(databasePath, error)) {
        return false;
    }

    // @TODO: ask if offsetFromUtc has changed since last time.
    int offsetFromUtc = settings.value("offsetFromUtc", INT_MIN).value<int>();
    if (offsetFromUtc == INT_MIN) {
        QDateTime dateTime = QDateTime::currentDateTime();
        offsetFromUtc = dateTime.offsetFromUtc();
        settings.setValue("offsetFromUtc", offsetFromUtc);
    }
    m_offsetFromUtc = offsetFromUtc * 1000; // It is stored as seconds in the file.

    settings.endGroup();

    m_currentDaySinceEpochUtc = getCompleteDaysSinceEpoch(getCurrentDateTimeUtc());

    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
            this, &ApplicationState::appAboutToQuit);

    return true;
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
    mainWindow()->onAppAboutToQuit();
}

ApplicationState g_app;
