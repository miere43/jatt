#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>
#include <QStringList>

enum class ActivityInfoFieldType;
struct ActivityInfo;

qint64 getCurrentDateTimeUtc();
inline qint64 getCompleteDaysSinceEpoch(qint64 utcMsecs) { return utcMsecs / 86400000; }

QString dbStringListToString(const QStringList& list);
QStringList dbStringToStringList(const QString& list);
QString dbActivityInfoFieldTypeVectorToString(const QVector<ActivityInfoFieldType>& vector);
QVector<ActivityInfoFieldType> dbStringToActivityInfoFieldTypeVector(const QString& list);
void addFieldToActivityInfo(ActivityInfo* info, QString fieldName, ActivityInfoFieldType type);

bool activityInfoLessThanByName(const ActivityInfo* a, const ActivityInfo* b);

#endif // UTILITIES_H
