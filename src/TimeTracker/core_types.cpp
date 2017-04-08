#include "core_types.h"

#include <QVariant>
#include <QDebug>

//bool IntervalTag::createFromDatabaseQuery(IntervalTag* target, QSqlQuery* query)
//{
//    Q_ASSERT(target);
//    Q_ASSERT(query);
//    bool ok = false;
//    target->id = query->value("id").toInt(&ok);
//    if (!ok) return false;
//    target->name = query->value("name").toString();
//    target->color = query->value("color").toInt(&ok);
//    if (!ok) return false;
//    return true;
//}

//QString tagsToString(Tag2** tags, size_t length)
//{
//    QString result;
//    size_t reserveSize = 0;

//    for (size_t i = 0; i < length; ++i)
//    {
//        reserveSize += tags[i]->name.length();
//        reserveSize += 2;
//    }

//    if (reserveSize >= 2)
//        reserveSize -= 2;

//    result.reserve(reserveSize);

//    for (size_t i = 0; i < length; ++i)
//    {
//        result.append(tags[i]->name);
//        if (i != length - 1)
//            result.append(QLatin1String(", "));
//    }

//    return result;
//}

QString createDurationStringFromMsecs(qint64 msecs)
{
    int secs  = msecs / 1000;
    int mins  = secs  / 60;
    int hours = mins  / 60;
    mins = mins % 60;
    secs = secs % 60;
    return QString("%1:%2:%3").arg(hours, 2, 10, QChar('0'))
                              .arg(mins,  2, 10, QChar('0'))
                              .arg(secs,  2, 10, QChar('0'));
}

QString ActivityInfo::formatActivity(Activity *activity)
{
    Q_ASSERT(activity);
    Q_ASSERT(activity->id > 0);
    Q_ASSERT(activity->info == this);

    return m_formatter.format(activity->fieldValues);
}

void ActivityInfo::updateFormatter()
{
    m_formatter.setFormat(displayFormat, fieldNames);
}

QString Activity::displayString()
{
    Q_ASSERT(id > 0);
    Q_ASSERT(info != nullptr);

    return info->formatActivity(this);
}
