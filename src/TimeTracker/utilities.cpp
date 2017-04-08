#include "utilities.h"
#include "application_state.h"

QString dbStringListToString(const QStringList& list)
{
    return list.join('\x1f');
}

QStringList dbStringToStringList(const QString &list)
{
    if (list.isNull() || list.isEmpty())
        return QStringList();
    return list.split('\x1f', QString::KeepEmptyParts);
}

void addFieldToActivityInfo(ActivityInfo* info, QString fieldName, ActivityInfoFieldType type)
{
    Q_ASSERT(info);
    Q_ASSERT(!fieldName.isNull() && !fieldName.isEmpty());

    info->fieldNames.append(fieldName);
    info->fieldTypes.append(type);

    Q_ASSERT(g_app.database()->saveActivityInfo(info));

    QVector<Activity*> activities;
    Q_ASSERT(g_app.database()->loadActivitiesAssociatedWithActivityInfo(info, &activities));

    for (Activity* activity : activities)
    {
        activity->fieldValues.append("");
        g_app.database()->saveActivity(activity);
    }
}

QString dbActivityInfoFieldTypeVectorToString(const QVector<ActivityInfoFieldType>& vector)
{
    QString result;
    result.reserve((2 * vector.count() + 1) - 1);
    for (int i = 0; i < vector.count(); ++i)
    {
        result.append(QString::number((int)vector[i]));
        if (i != vector.count() - 1)
            result.append('\x1f');
    }
    return result;
}

QVector<ActivityInfoFieldType> dbStringToActivityInfoFieldTypeVector(const QString& list)
{
    QVector<ActivityInfoFieldType> result;
    QStringList types = list.split('\x1f');
    result.reserve(types.count());
    for (int i = 0; i < types.count(); ++i)
    {
        result.append((ActivityInfoFieldType)types[i].toInt());
    }
    return result;
}
