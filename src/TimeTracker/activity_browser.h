#ifndef ACTIVITY_BROWSER_H
#define ACTIVITY_BROWSER_H

#include <QMainWindow>
#include <QVector>
#include "activity_table_model.h"

namespace Ui {
class ActivityBrowser;
}

class ActivityBrowser : public QMainWindow
{
    Q_OBJECT

public:
    explicit ActivityBrowser(QWidget *parent = nullptr);
    ~ActivityBrowser();

private slots:
    void executeSearchAction();
private:
    Ui::ActivityBrowser *ui;
    QVector<Activity *> m_activities;
    ActivityTableModel * m_activityTableModel = nullptr;


    void search(QString query);
};

#endif // ACTIVITY_BROWSER_H
