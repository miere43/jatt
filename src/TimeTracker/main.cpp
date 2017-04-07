#include "mainwindow.h"
#include <QApplication>
#include "application_state.h"
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if (!g_app.openDatabase())
    {
        QMessageBox::information(nullptr, "Time Tracker Error",
                                 "Unable to open database. Program will be terminated.",
                                 QMessageBox::Ok);
        a.exit(1);
        return 1; // I just don't know if a.exit(1) terminates program.
    }

    qDebug() << sizeof(QDateTime);

    g_app.initialize();
    MainWindow w;
    w.show();

    return a.exec();
}
