#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include <QSqlQuery>
#include <QVector>
#include <QHash>
#include "date_time.h"

// old stuff
struct Recording;

struct Session {
    qint64 id = -1;
    QString name = "";
    QString note = "";
    qint64 timezone = 0;
    qint64 created = 0;
    QVector<Recording*> recordings;

    void dumpToQDebug() const;
};

struct Interval2 {
    qint64 startTime;
    qint64 endTime;
};

struct Tag2 {
    qint64 id = -1;
    // Color of the tag. Stored as RGBA value, alpha component is unused.
    qint64 color = -1;
    QString name = "";
    QString note = "";
};

struct Recording {
    qint64 id = -1;
    Session* session = nullptr;
    QString name = "";
    QString note = "";
    qint64 startTime = 0;
    qint64 endTime = 0;
    QVector<Interval2> intervals;
    QVector<Tag2*> tags;

    void dumpToQDebug() const;
};

// old stuff end

struct Interval {
    DateTime startTime;
    DateTime endTime;
};

struct ActivityInfo
{
    qint64 id = 0;
    qint64 color = 0;
    QStringList fieldNames;
    QStringList fieldTypes;
    QString displayFormat;
    QString displayRules;
};

struct Activity
{
    qint64 id = 0;
    ActivityInfo* info = nullptr;
    QStringList fieldValues;
    DateTime startTime;
    DateTime endTime;
    QVector<Interval> intervals;
};

struct Tag
{
    qint64 id = 0;
    QString name;
    QString note;
};

struct UserProperties
{
    qint64 firstActivityDay = 0;
    QHash<QString, QString> customProperties;
};

int clamp(int value, int min, int max);

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
QString tagsToString(Tag2** tags, size_t length);

#endif // CORE_TYPES_H
