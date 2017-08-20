#include "statistics_dialog.h"
#include "ui_statistics_dialog.h"
#include "error_macros.h"
#include "application_state.h"
#include "utilities.h"

#include <algorithm>
#include <QMessageBox>
#include <QHash>



static inline qint64 qint64_clamp(qint64 value, qint64 min, qint64 max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static inline qint64 clampActivityDuration(const Activity* activity, qint64 min, qint64 max)
{
    qint64 total = 0i64;
    for (const Interval& interval : activity->intervals)
    {
        qint64 intervalMin = qint64_clamp(interval.startTime, min, max);
        qint64 intervalMax = qint64_clamp(interval.endTime, min, max);
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
    Q_UNUSED(parent);
    return m_items.count();
}

int StatisticsTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (m_items.count() == 0) return 0;
    return 2;
}

QVariant StatisticsTableModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();

    const StatisticsTableItem& item = m_items[index.row()];
    switch (index.column())
    {
        case 0:
        {
            return item.name;
        }
        case 1:
        {
            return formatDuration(item.time);
        }
        default:
        {
            ERR_VERIFY_V(false, QVariant());
        }
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
            ERR_VERIFY_V(false, QVariant());
        }
        else if (orientation == Qt::Vertical)
        {
            return QVariant();
        }
        ERR_VERIFY_V(false, QVariant());
    }

    return QVariant();
}

static inline bool tableItemTimeLessThanAsc(const StatisticsTableItem& a, const StatisticsTableItem& b)
{
    return a.time < b.time;
}

static inline bool tableItemTimeLessThanDesc(const StatisticsTableItem& a, const StatisticsTableItem& b)
{
    return a.time > b.time;
}

static inline bool tableItemNameLessThanAsc(const StatisticsTableItem& a, const StatisticsTableItem& b)
{
    // @TODO: maybe this isn't right way to do it.
    return a.name.toLower() < b.name.toLower();
}

static inline bool tableItemNameLessThanDesc(const StatisticsTableItem& a, const StatisticsTableItem& b)
{
    return a.name.toLower() > b.name.toLower();
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

StatisticsDialog::StatisticsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StatisticsDialog)
{
    ui->setupUi(this);
    m_tableModel = new StatisticsTableModel(this);
    ui->infoTable->setModel(m_tableModel);

    // @TODO: ugly
    this->on_selectRangeCombobox_currentIndexChanged(ui->selectRangeCombobox->currentIndex());
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
            return;
        }
        case 1:
        {
            // @TODO: all stuff below probably doesn't care about local offset from UTC...
            // Week
            QDateTime now = QDateTime::currentDateTimeUtc();
            qint64 currentDayLocal = g_app.currentDaySinceEpochLocal();
            qint64 dateWeekDay = now.date().dayOfWeek() - 1; // because dayOfWeek() starts from one.
            qint64 weekStartLocal = currentDayLocal - dateWeekDay;

            qint64 startTime = weekStartLocal * 86400000LL;
            qint64 endTime = currentDayLocal * 86400000LL + 86400000LL;
            calcStatisticsForTimeRange(startTime, endTime);
            return;
        }
        case 2:
        {
            // Month
            QDateTime now = QDateTime::currentDateTimeUtc();
            qint64 currentDayLocal = g_app.currentDaySinceEpochLocal();
            qint64 dateMonthDay = now.date().day() - 1; // because day() starts from one.
            qint64 monthStartLocal = currentDayLocal - dateMonthDay;

            qint64 startTime = monthStartLocal * 86400000LL;
            qint64 endTime = currentDayLocal * 86400000LL + 86400000LL;
            calcStatisticsForTimeRange(startTime, endTime);
            return;
        }
        case 3:
        {
            // Lifetime
            qint64 startTime = 0i64;
            qint64 endTime = INT64_MAX;
            calcStatisticsForTimeRange(startTime, endTime);
            return;
        }
    default:
        ERR_VERIFY(false);
    }
}

void StatisticsDialog::calcStatisticsForTimeRange(qint64 startTime, qint64 endTime)
{
    ERR_VERIFY(startTime >= 0);
    ERR_VERIFY(endTime >= 0);
    ERR_VERIFY(startTime <= endTime);

    QList<ActivityInfo*> activityInfos = g_app.database()->activityInfos();
    if (activityInfos.length() == 0)
    {
        QMessageBox::critical(this, "Statistics Error", "No activity infos available to count statistics.");
        return;
    }

    QVector<Activity*> activities;
    if (!g_app.database()->loadActivitiesBetweenStartAndEndTime(&activities, startTime, endTime))
    {
        QMessageBox::critical(this, "Statistics Error", "Unable to load activities for statistics calculation.");
        return;
    }

    QHash<ActivityInfo*, qint64> count;
    count.reserve(activityInfos.length());
    for (ActivityInfo* activityInfo : activityInfos)
    {
        count.insert(activityInfo, 0);
    }

    for (const Activity* activity : activities)
    {
        auto it = count.find(activity->info);
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
