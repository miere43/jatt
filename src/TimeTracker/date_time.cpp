#include "date_time.h"
#include "windows.h"
#include <QDateTime>

DateTime::DateTime(quint64 milliseconds)
{
    this->ticks = milliseconds;
}

DateTime getCurrentDateTime()
{
    QDateTime time = QDateTime::currentDateTimeUtc();
    return DateTime(time.currentMSecsSinceEpoch());
}
