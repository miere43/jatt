#ifndef DATE_TIME_H
#define DATE_TIME_H

#include <QtGlobal>

class DateTime
{
public:
    DateTime(quint64 milliseconds);

    inline int completeDaysSinceEpoch() const { return ticks / 86400000; }

    // Time since 1 January, 1960.
    quint64 ticks;
};

DateTime getCurrentDateTime();

#endif // DATE_TIME_H
