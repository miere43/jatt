#include <QApplication>
#include <QMessageBox>

#include "application_state.h"
#include "mainwindow.h"
#include "block_allocator.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if (!g_app.openDatabase())
    {
        QMessageBox::information(nullptr, "Time Tracker Error",
                                 "Unable to open database. Program will be terminated.",
                                 QMessageBox::Ok);
        a.exit(1);
        Q_UNREACHABLE();
    }

    g_app.initialize();
    MainWindow w;
    w.show();

    return a.exec();
}
