#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include <QSqlQuery>
#include <QVector>
#include <QHash>
#include "date_time.h"
#include "display_format.h"

struct Activity;

struct Interval
{
    qint64 startTime;
    qint64 endTime;
};

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

    void updateFormatter();
    QString formatActivity(Activity* activity);
private:
    DisplayFormat m_formatter;
};

struct Activity
{
    qint64 id;
    ActivityInfo* info;
    QStringList fieldValues;
    qint64 startTime;
    qint64 endTime;
    QVector<Interval> intervals;

    QString displayString();
};

struct Tag
{
    qint64 id = 0;
    QString name;
    QString note;
};

struct UserProperties
{
    qint64 firstActivityDayUtc = 0;
    qint64 localTimeZoneOffsetFromUtc = 0;
    QHash<QString, QString> customProperties;
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
