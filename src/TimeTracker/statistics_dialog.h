#ifndef STATISTICS_DIALOG_H
#define STATISTICS_DIALOG_H

#include <QDialog>


namespace Ui {
class StatisticsDialog;
}

class StatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatisticsDialog(QWidget *parent = 0);
    ~StatisticsDialog();

private slots:
    void on_selectRangeCombobox_currentIndexChanged(int index);

private:
    Ui::StatisticsDialog *ui;


    void calcStatisticsForTimeRange(qint64 startTime, qint64 endTime);
};

#endif // STATISTICS_DIALOG_H
