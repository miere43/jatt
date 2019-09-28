#include "application_state.h"
#include "error_macros.h"
#include "mainwindow.h"
#include "utilities.h"

#include <QCoreApplication>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QMessageBox>
#include <QSettings>
#include <QVariant>
#include <QDebug>
#include <QDir>

ApplicationState::ApplicationState(QObject *parent)
    : QObject(parent)
{
#ifdef APP_DEBUG_DB
    appTitle = QStringLiteral("(Debug) Qt Time Tracker");
#else
    appTitle = QStringLiteral("Qt Time Tracker");
#endif
}

void errorListener(const char* function, const char* file, int line, const char* message, void* userdata) {
    Q_UNUSED(userdata)

    QString errorMessage = QString(QStringLiteral("Error at \"%1:%2\" in %3: \"%4\""))
            .arg(file).arg(line).arg(function).arg(message);

#ifdef QT_DEBUG
    qDebug().noquote() << errorMessage;
#else
    QMainWindow* window = g_app.mainWindow();
    QMessageBox::critical(window, "Error", "Application has recorded an error:\n" + errorMessage + "\n\nPlease forward this message to the developers.");
#endif
}

bool ApplicationState::initialize(QString* error)
{
    if (m_initialized)
        return true;

    addErrorListener(errorListener, reinterpret_cast<void*>(this));
    ERR_VERIFY_NULL_V(error, false);

    QSettings settings;
    settings.beginGroup(QStringLiteral("main"));

    QString databasePath = settings.value("databasePath").value<QString>();
    if (databasePath.isEmpty() || !QFile::exists(databasePath)) {
        QString appDirPath = QCoreApplication::applicationDirPath();
        QDir appDir(appDirPath);
        appDir.mkdir("data");
        if (!appDir.cd("data")) {
            *error = QString("Unable to create directory 'data' in '%1'.").arg(appDirPath);
            return false;
        }
        databasePath = appDir.filePath("user.s3db");
        settings.setValue("databasePath", databasePath);
    }

    qDebug() << "Using database" << databasePath;
    if (!m_databaseManager.connectToDatabase(databasePath, error)) {
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

    m_initialized = true;
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
