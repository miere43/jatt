#ifndef DATE_TIME_H
#define DATE_TIME_H

#include <QtGlobal>

qint64 getCurrentDateTimeUtc();
inline qint64 getCompleteDaysSinceEpoch(qint64 utcMsecs) { return utcMsecs / 86400000; }

#endif // DATE_TIME_H
