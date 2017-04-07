#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include <QSqlQuery>
#include <QVector>

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

struct Interval {
    qint64 startTime;
    qint64 endTime;
};

struct Tag {
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
    QVector<Interval> intervals;
    QVector<Tag*> tags;

    void dumpToQDebug() const;
};

int clamp(int value, int min, int max);

//struct IntervalTag {
//    int id;
//    QString name;
//    int color;

//    static bool createFromDatabaseQuery(IntervalTag* target, QSqlQuery* query);
//};

//struct Interval {
//    int id;
//    QString name;
//    QString note;
//    Session* session;
//    qint64 timestamp;
//    qint64 duration;
//    QVector<IntervalTag*> tags;

//    // @TODO: Add GUI-related stuff here (maybe):
//    // int listModelPos; // position in intervalListModel associated with session.
//                         // removes linear search for sendDataChangedHint.
//};

//struct Session {
//    int id = -1;
//    QString name = "";
//    QString note = "";
//    int created = 0;
//    int timezone = 0;
//    QVector<Interval*> intervals;
//};

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
QString tagsToString(Tag** tags, size_t length);

#endif // CORE_TYPES_H
