#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include <QSqlQuery>
#include <QVector>
#include <QHash>

#include "utilities.h"

struct Activity;

struct Interval
{
    qint64 startTime;
    qint64 endTime;

    bool isInTimePeriod(qint64 start, qint64 end) const;
    inline qint64 duration() const { return endTime - startTime; }
};

struct ActivityCategory
{
    qint64 id = 0;
    qint64 color = 0;
    QString name;
};

struct Activity
{
    qint64 id;
    ActivityCategory* category;
    qint64 startTime;
    qint64 endTime;
    QString name;
    QString note;
    QVector<Interval> intervals;

    bool belongsToTimePeriod(qint64 startTime, qint64 endTime);
    void updateStartAndEndTime();

    qint64 duration() const;
};

#endif // CORE_TYPES_H
