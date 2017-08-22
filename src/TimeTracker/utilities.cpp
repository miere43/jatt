#include "utilities.h"
#include "application_state.h"
#include "error_macros.h"

#include <QDateTime>

qint64 getCurrentDateTimeUtc()
{
    QDateTime time = QDateTime::currentDateTimeUtc();
    return time.currentMSecsSinceEpoch();
}

QString formatDuration(qint64 time)
{
    qint64 secs = time / 1000;
    qint64 mins = secs / 60;
    qint64 hrs  = mins / 60;
    qint64 msecs = time % 1000;
    mins = mins % 60;
    secs = secs % 60;

    return QString(QStringLiteral("%1:%2:%3.%4"))
            .arg(hrs, 2, 10, QLatin1Char('0'))
            .arg(mins, 2, 10, QLatin1Char('0'))
            .arg(secs, 2, 10, QLatin1Char('0'))
            .arg(msecs, 4, 10, QLatin1Char('0'));
}

bool activityCategoryLessThanByName(const ActivityCategory* a, const ActivityCategory* b) {
    ERR_VERIFY_NULL_V(a, false);
    ERR_VERIFY_NULL_V(b, false);

    return a->name < b->name;
}

