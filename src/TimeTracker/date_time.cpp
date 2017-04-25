#include "date_time.h"
#include "windows.h"
#include <QDateTime>

qint64 getCurrentDateTimeUtc()
{
    QDateTime time = QDateTime::currentDateTimeUtc();
    return time.currentMSecsSinceEpoch();
}
