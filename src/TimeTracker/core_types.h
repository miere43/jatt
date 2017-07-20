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

// bool operator==(const Interval& lhs, const Interval& rhs) const;

enum class ActivityInfoFieldType
{
    String,
};

struct ActivityInfo
{
    qint64 id = 0;
    qint64 color = 0;
    QString name;
    QStringList fieldNames;
    QVector<ActivityInfoFieldType> fieldTypes;
    QString displayFormat;
    QString displayRules;
};

struct Activity
{
    qint64 id;
    ActivityInfo* info;
    qint64 startTime;
    qint64 endTime;
    QString name;
    QString note;
    QVector<Interval> intervals;

    bool belongsToTimePeriod(qint64 startTime, qint64 endTime);
    void updateStartAndEndTime();

    qint64 duration() const;
};

struct Tag
{
    qint64 id = 0;
    QString name;
    QString note;
};

/**
 * @brief Creates string in format hh:mm:ss from milliseconds.
 * @param msecs
 * @return 'msecs' formatted in hh:mm:ss format.
 */
QString createDurationStringFromMsecs(qint64 msecs);

/**
 * @brief Creates string which contains tag names seperated by comma.
 * @param tags Pointer to contigious array of elements of type 'Tag*'.
 * @param length Length of 'tags' array, in elements.
 * @return String of tag names seperated by comma.
 */
//QString tagsToString(Tag2** tags, size_t length);

#endif // CORE_TYPES_H
