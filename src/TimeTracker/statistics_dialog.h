#ifndef STATISTICS_DIALOG_H
#define STATISTICS_DIALOG_H

#include <QDialog>
#include <QAbstractTableModel>

namespace Ui {
class StatisticsDialog;
}

struct StatisticsTableItem
{
    QString name;
    qint64  time;
};

class StatisticsTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit StatisticsTableModel(QObject* parent = 0);
    void setItems(QVector<StatisticsTableItem> items);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    void sort(int column, Qt::SortOrder order) override;

private:
    QVector<StatisticsTableItem> m_items;
};

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
    QVector<StatisticsTableItem> m_items;
    StatisticsTableModel * m_tableModel = 0;
    bool m_isFirstStatsCalc = true;

    void calcStatisticsForTimeRange(qint64 startTime, qint64 endTime);
};






#endif // STATISTICS_DIALOG_H
