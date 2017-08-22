#include "core_types.h"
#include "error_macros.h"

#include <QVariant>
#include <QDebug>

bool Interval::isInTimePeriod(qint64 start, qint64 end) const
{
    return ((start >= startTime && end <= endTime) ||
            (start >= startTime && end >= startTime && endTime >= start) ||
            (start <= startTime && end >= startTime));
}

QString createDurationStringFromMsecs(qint64 msecs)
{
    int secs  = msecs / 1000;
    int mins  = secs  / 60;
    int hours = mins  / 60;
    mins = mins % 60;
    secs = secs % 60;
    return QString("%1:%2:%3").arg(hours, 2, 10, QChar('0'))
                              .arg(mins,  2, 10, QChar('0'))
                              .arg(secs,  2, 10, QChar('0'));
}

void Activity::updateStartAndEndTime()
{
    qint64 min = INT64_MAX, max = INT64_MIN;
    if (intervals.count() == 0) return;
    for (const Interval& interval : intervals)
    {
        if (interval.startTime < min)
            min = interval.startTime;
        if (interval.endTime > max)
            max = interval.endTime;
    }
    startTime = min;
    endTime = max;
}

bool Activity::belongsToTimePeriod(qint64 startTime, qint64 endTime)
{
    if (intervals.count() == 0) {
        return (startTime >= this->startTime && endTime <= this->endTime) ||
                (startTime >= this->startTime && endTime >= this->startTime && this->endTime >= startTime) ||
                (startTime <= this->startTime && endTime >= this->startTime);
    }
    for (const Interval& interval : intervals)
    {
        /* this one is better i guess
        (startTime >= interval.startTime && endTime >= interval.startTime && interval.endTime >= startTime)
        ||
        (startTime <= interval.startTime && endTime >= interval.startTime);
        */
       if ((startTime >= interval.startTime && endTime <= interval.endTime) ||
           (startTime >= interval.startTime && endTime >= interval.startTime && interval.endTime >= startTime) ||
           (startTime <= interval.startTime && endTime >= interval.startTime))
       {
           return true;
       }
    }

    return false;
}


qint64 Activity::duration() const
{
    qint64 duration = 0;
    int length = intervals.length();
    for (int i = 0; i < length; ++i) {
        duration += intervals[i].duration();
    }
    // qDebug() << field(0) << duration / (1000.0 * 60.0);
    return duration;
}
