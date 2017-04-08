#ifndef DATE_TIME_H
#define DATE_TIME_H

#include <QtGlobal>

class DateTime
{
public:
    DateTime();
    DateTime(quint64 milliseconds);

    inline int completeDaysSinceEpoch() const { return ticks / 86400000; }
    inline qint64 msecs() const { return ticks; }

    // Local time offset.
    // quint64 offset;
    // Time since 1 January, 1960.
    qint64 ticks;
};

qint64 getCurrentDateTimeUtc();
inline qint64 getCompleteDaysSinceEpoch(qint64 utcMsecs) { return utcMsecs / 86400000; }

#endif // DATE_TIME_H
