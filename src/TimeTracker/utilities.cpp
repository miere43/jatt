#include "utilities.h"

QString stringListToString(const QStringList& list)
{
    // @Speed
    QString result;
    for (const QString& value : list)
    {
        result.append(value);
        result.append('\x1f');
    }
    return result;
}

QStringList stringToStringList(const QString &list)
{
    return list.split('\x1f', QString::KeepEmptyParts);
}
