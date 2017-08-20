#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>

enum class ActivityInfoFieldType;
struct ActivityInfo;

qint64 getCurrentDateTimeUtc();
inline qint64 getCompleteDaysSinceEpoch(qint64 utcMsecs) { return utcMsecs / 86400000; }

// Formats milliseconds as HH:MM:SS.MS
QString formatDuration(qint64 milliseconds);

bool activityInfoLessThanByName(const ActivityInfo * a, const ActivityInfo * b);

#endif // UTILITIES_H
