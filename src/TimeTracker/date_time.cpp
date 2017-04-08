#include "date_time.h"
#include "windows.h"
#include <QDateTime>

DateTime::DateTime()
{
    this->ticks = 0;
}

DateTime::DateTime(quint64 milliseconds)
{
    this->ticks = milliseconds;
}

qint64 getCurrentDateTimeUtc()
{
    QDateTime time = QDateTime::currentDateTimeUtc();
    return time.currentMSecsSinceEpoch();
}
