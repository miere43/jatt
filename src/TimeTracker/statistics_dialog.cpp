#include "statistics_dialog.h"
#include "ui_statistics_dialog.h"
#include "error_macros.h"
#include "application_state.h"
#include "utilities.h"

#include <QMessageBox>
#include <QHash>

StatisticsDialog::StatisticsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StatisticsDialog)
{
    ui->setupUi(this);

    this->on_selectRangeCombobox_currentIndexChanged(ui->selectRangeCombobox->currentIndex());
}

StatisticsDialog::~StatisticsDialog()
{
    delete ui;
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

static QString formatMs(qint64 time)
{
    qint64 secs = time / 1000;
    qint64 mins = secs / 60;
    qint64 hrs  = mins / 60;
    qint64 msecs = time % 1000;
    mins = mins % 60;
    secs = secs % 60;

    return QString(QStringLiteral("%1:%2:%3.%4"))
            .arg(hrs, 2, 10, QLatin1Char('0'))
            .arg(mins, 2, 10, QLatin1Char('0'))
            .arg(secs, 2, 10, QLatin1Char('0'))
            .arg(msecs, 4, 10, QLatin1Char('0'));
}

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

void StatisticsDialog::calcStatisticsForTimeRange(qint64 startTime, qint64 endTime)
{
    ERR_VERIFY(startTime >= 0);
    ERR_VERIFY(endTime >= 0);
    ERR_VERIFY(startTime <= endTime);

    // @TODO: we shouldn't count activity duration before 'startTime' or 'endTime'!!!

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

    QString output;
    qint64 total = 0LL;
    for (auto it = count.cbegin(); it != count.cend(); ++it)
    {
        qint64 duration = it.value();
        output.append(it.key()->name);
        output.append(QStringLiteral(": "));
        output.append(formatMs(duration));
        output.append(QStringLiteral("\n"));
        total += duration;
    }
    output.append(QStringLiteral("Total: "));
    output.append(formatMs(total));

    ui->displayLabel->setText(output);
}
