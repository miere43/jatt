#include "utilities.h"
#include "application_state.h"
#include "error_macros.h"

#include <QDateTime>

qint64 getCurrentDateTimeUtc()
{
    QDateTime time = QDateTime::currentDateTimeUtc();
    return time.currentMSecsSinceEpoch();
}

bool activityInfoLessThanByName(const ActivityInfo* a, const ActivityInfo* b) {
    ERR_VERIFY_NULL_V(a, false);
    ERR_VERIFY_NULL_V(b, false);

    return a->name < b->name;
}
