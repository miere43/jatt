#include "statistics_dialog.h"
#include "ui_statistics_dialog.h"
#include "error_macros.h"
#include "application_state.h"
#include "utilities.h"

#include <algorithm>
#include <QtGlobal>
#include <QMessageBox>
#include <QHash>


static inline qint64 clampActivityDuration(const Activity* activity, qint64 min, qint64 max)
{
    qint64 total = 0LL;
    for (const Interval& interval : activity->intervals)
    {
        qint64 intervalMin = qBound(min, interval.startTime, max);
        qint64 intervalMax = qBound(min, interval.endTime,   max);
        total += intervalMax - intervalMin;
    }
    return total;
}

StatisticsTableModel::StatisticsTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}

void StatisticsTableModel::setItems(QVector<StatisticsTableItem> items)
{
    beginResetModel();
    m_items = items;

    StatisticsTableItem total;
    total.name = QStringLiteral("Total");
    total.time = 0;
    for (const StatisticsTableItem& item : items)
    {
        total.time += item.time;
    }

    m_items.append(total);
    endResetModel();
}

int StatisticsTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_items.count();
}

int StatisticsTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (m_items.count() == 0) return 0;
    return 2;
}

QVariant StatisticsTableModel::data(const QModelIndex &index, int role) const
{
    const StatisticsTableItem& item = m_items[index.row()];

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
            case 0:  return item.name;
            case 1:  return formatDuration(item.time, true);
        }
    }
    else if (role == Qt::UserRole)
    {
        return item.time;
    }
    else
    {
        return QVariant();
    }

    ERR_VERIFY_V(false, QVariant());
}

QVariant StatisticsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            if (section == 0) return QStringLiteral("Activities");
            if (section == 1) return QStringLiteral("Time");
        }
        else if (orientation == Qt::Vertical)
        {
            return QVariant();
        }
        ERR_VERIFY_V(false, QVariant());
    }

    return QVariant();
}

static inline bool tableItemTimeLessThanAsc(const StatisticsTableItem & a, const StatisticsTableItem & b)
{
    return a.time < b.time;
}

static inline bool tableItemTimeLessThanDesc(const StatisticsTableItem & a, const StatisticsTableItem & b)
{
    return a.time > b.time;
}

static inline bool tableItemNameLessThanAsc(const StatisticsTableItem & a, const StatisticsTableItem & b)
{
    return QString::compare(a.name, b.name, Qt::CaseInsensitive) < 0;
}

static inline bool tableItemNameLessThanDesc(const StatisticsTableItem & a, const StatisticsTableItem & b)
{
    return QString::compare(a.name, b.name, Qt::CaseInsensitive) > 0;
}

void StatisticsTableModel::sort(int column, Qt::SortOrder order)
{
    if (column < 0 || column > 1 || m_items.count() <= 1) return;
    emit layoutAboutToBeChanged();

    // We assume that last item is 'Total' row, don't sort it.
    if (column == 0)
    {
        std::sort(m_items.begin(), m_items.end() - 1,
                  order == Qt::AscendingOrder ? tableItemNameLessThanAsc : tableItemNameLessThanDesc);
    }
    else if (column == 1)
    {
        std::sort(m_items.begin(), m_items.end() - 1,
                  order == Qt::AscendingOrder ? tableItemTimeLessThanAsc : tableItemTimeLessThanDesc);
    }

    emit layoutChanged();
}

StatisticsDialog::StatisticsDialog(QWidget * parent) :
    QDialog(parent),
    ui(new Ui::StatisticsDialog)
{
    ui->setupUi(this);

    m_tableModel = new StatisticsTableModel(this);
    ui->infoTable->setModel(m_tableModel);

    // @TODO: ugly
    this->on_selectRangeCombobox_currentIndexChanged(ui->selectRangeCombobox->currentIndex());

    auto selectionModel = ui->infoTable->selectionModel();
    QObject::connect(selectionModel, &QItemSelectionModel::selectionChanged,
                     this, &StatisticsDialog::on_infoTable_selectionModel_selectionChanged);
}

StatisticsDialog::~StatisticsDialog()
{
    delete ui;
    delete m_tableModel;
}

void StatisticsDialog::on_selectRangeCombobox_currentIndexChanged(int index)
{
    switch (index)
    {
        case 0:
        {
            // Today
            qint64 startTime = g_app.currentDaySinceEpochUtc() * 86400000LL - g_app.localOffsetFromUtc();
            qint64 endTime = startTime + 86400000LL;

            calcStatisticsForTimeRange(startTime, endTime);
            break;
        }
        case 1:
        {
            // Yesterday
            qint64 startTime = (g_app.currentDaySinceEpochUtc() - 1) * 86400000LL - g_app.localOffsetFromUtc();
            qint64 endTime = startTime + 86400000LL;

            calcStatisticsForTimeRange(startTime, endTime);
            break;
        }
        case 2:
        {
            // This month
            QDate now = QDate::currentDate();

            QDateTime monthStart = QDateTime(QDate(now.year(), now.month(), 1), QTime(), Qt::UTC);
            QDateTime monthEnd = monthStart.addMonths(1).addMSecs(-1);

            qint64 offset = g_app.localOffsetFromUtc();

            qint64 startTime = monthStart.toMSecsSinceEpoch() - offset;
            qint64 endTime = monthEnd.toMSecsSinceEpoch() - offset;

            calcStatisticsForTimeRange(startTime, endTime);
            break;
        }
        case 3:
        {
            // This year
            QDate now = QDate::currentDate();

            QDateTime yearStart = QDateTime(QDate(now.year(), 1, 1), QTime(), Qt::UTC);
            QDateTime yearEnd = yearStart.addYears(1).addMSecs(-1);

            qint64 offset = g_app.localOffsetFromUtc();

            qint64 startTime = yearStart.toMSecsSinceEpoch() - offset;
            qint64 endTime = yearEnd.toMSecsSinceEpoch() - offset;

            calcStatisticsForTimeRange(startTime, endTime);
            break;
        }
        case 4:
        {
            // Last year
            QDate now = QDate::currentDate();

            QDateTime yearStart = QDateTime(QDate(now.year() - 1, 1, 1), QTime(), Qt::UTC);
            QDateTime yearEnd = yearStart.addYears(1).addMSecs(-1);

            qint64 offset = g_app.localOffsetFromUtc();

            qint64 startTime = yearStart.toMSecsSinceEpoch() - offset;
            qint64 endTime = yearEnd.toMSecsSinceEpoch() - offset;

            calcStatisticsForTimeRange(startTime, endTime);
            break;
        }
        case 5:
        {
            // Lifetime
            calcStatisticsForTimeRange(0LL, INT64_MAX);
            break;
        }
        default:
            ERR_VERIFY(false);
    }

    updateSelectedRowsTotalTimeLabel();
}

void StatisticsDialog::calcStatisticsForTimeRange(qint64 startTime, qint64 endTime)
{
    ERR_VERIFY(startTime >= 0);
    ERR_VERIFY(endTime >= 0);
    ERR_VERIFY(startTime <= endTime);

    QList<ActivityCategory*> categories = g_app.database()->activityCategories();
    if (categories.length() == 0)
    {
        QMessageBox::critical(this, "Statistics Error", "No categories available to count statistics.");
        return;
    }

    QVector<Activity*> activities;
    if (!g_app.database()->loadActivitiesBetweenStartAndEndTime(&activities, startTime, endTime))
    {
        QMessageBox::critical(this, "Statistics Error", "Unable to load activities for statistics calculation.");
        return;
    }

    QHash<ActivityCategory*, qint64> count;
    count.reserve(categories.length());
    for (ActivityCategory* category : categories)
    {
        count.insert(category, 0);
    }

    for (const Activity* activity : activities)
    {
        auto it = count.find(activity->category);
        if (it != count.end())
        {
            qint64 storedDuration = it.value();
            it.value() = storedDuration + clampActivityDuration(activity, startTime, endTime);
        }
    }

    m_items.clear();
    for (auto it = count.cbegin(); it != count.cend(); ++it)
    {
        qint64 duration = it.value();
        if (duration > 0)
        {
            m_items.append(StatisticsTableItem { it.key()->name, duration });
        }
    }

    m_tableModel->setItems(m_items);

    QHeaderView* headerView = ui->infoTable->horizontalHeader();
    if (m_isFirstStatsCalc)
    {
        m_isFirstStatsCalc = false;
        headerView->setSortIndicator(1, Qt::DescendingOrder);
    }
    else
    {
        m_tableModel->sort(headerView->sortIndicatorSection(), headerView->sortIndicatorOrder());
    }
}

void StatisticsDialog::on_infoTable_selectionModel_selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

    updateSelectedRowsTotalTimeLabel();
}

void StatisticsDialog::updateSelectedRowsTotalTimeLabel()
{
    QModelIndexList indexes = ui->infoTable->selectionModel()->selection().indexes();
    if (indexes.count() == 0)
    {
        ui->selectedRowsTimeLabel->setText(QStringLiteral("Selected rows total: no rows selected"));
        return;
    }

    qint64 totalDuration = 0;

    for (const QModelIndex& index : indexes)
    {
        if (index.column() != 0)  continue;

        qint64 rowDuration = index.data(Qt::UserRole).value<qint64>();
        totalDuration += rowDuration;
    }

    ui->selectedRowsTimeLabel->setText(
        QString("Selected rows total: %1")
        .arg(formatDuration(totalDuration, true))
    );
}
