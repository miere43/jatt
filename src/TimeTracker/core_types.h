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

    bool isInTimePeriod(qint64 startTime, qint64 endTime) const;
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
    qint64 id = 0;
    ActivityCategory * category = nullptr;
    qint64 startTime = 0;
    qint64 endTime = 0;
    QString name;
    QString note;
    QVector<Interval> intervals;

    void updateStartAndEndTime();

    bool belongsToTimePeriod(qint64 startTime, qint64 endTime) const;
    qint64 duration() const;
};

#endif // CORE_TYPES_H
