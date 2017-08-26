#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>

struct Activity;
struct ActivityCategory;

qint64 getCurrentDateTimeUtc();
inline qint64 getCompleteDaysSinceEpoch(qint64 utcMsecs) { return utcMsecs / 86400000; }

template<typename T>
T clamp(T value, T min, T max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}


// Formats milliseconds as HH:MM:SS.MS
QString formatDuration(qint64 milliseconds, bool withMilliseconds);
bool activityCategoryLessThanByName(const ActivityCategory * a, const ActivityCategory * b);

#endif // UTILITIES_H
