#include "application_state.h"
#include "error_macros.h"
#include "mainwindow.h"
#include "utilities.h"

#include <QDebug>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QVariant>
#include <QCoreApplication>
#include <QSettings>

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
    ERR_VERIFY_NULL_V(error, false);

    QSettings settings;
    settings.beginGroup(QStringLiteral("main"));

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

    // @TODO: ask if localOffsetFromUtc has changed since last time.
    int localOffsetFromUtc = settings.value("localOffsetFromUtc", INT_MIN).value<int>();
    if (localOffsetFromUtc == INT_MIN) {
        QDateTime dateTime = QDateTime::currentDateTime();
        localOffsetFromUtc = dateTime.offsetFromUtc();
        settings.setValue("localOffsetFromUtc", localOffsetFromUtc);
    }
    m_localOffsetFromUtc = localOffsetFromUtc * 1000; // It is stored as seconds in the file.

    settings.endGroup();

//    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
//            this, &ApplicationState::appAboutToQuit);

    return true;
}

DatabaseManager* ApplicationState::database() {
    return &m_databaseManager;
}

MainWindow* ApplicationState::mainWindow() {
    return m_mainWindow;
}

void ApplicationState::setMainWindow(MainWindow* mainWindow) {
    m_mainWindow = mainWindow;
}

//void ApplicationState::appAboutToQuit()
//{
//}

ApplicationState g_app;
