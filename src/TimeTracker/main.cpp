#include <QApplication>
#include <QMessageBox>
#include <QSettings>

#include "application_state.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QCoreApplication::setOrganizationName("miere");
    QCoreApplication::setOrganizationDomain("miere.ru");
    QCoreApplication::setApplicationName(
#ifdef QT_DEBUG
    "Time Tracker debug"
#else
    "Time Tracker"
#endif
    );

    QString error;
    if (!g_app.initialize(&error)) {
        QMessageBox::critical(nullptr, "Error", "Unable to initialize program, error: \"" + error + "\"", QMessageBox::Ok);
        return 1;
    } else {
        MainWindow w;
        w.show();

        return a.exec();
    }
    Q_UNREACHABLE();
}
