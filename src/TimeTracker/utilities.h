#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>
#include <QStringList>

#include "core_types.h"

QString dbStringListToString(const QStringList& list);
QStringList dbStringToStringList(const QString& list);
QString dbActivityInfoFieldTypeVectorToString(const QVector<ActivityInfoFieldType>& vector);
QVector<ActivityInfoFieldType> dbStringToActivityInfoFieldTypeVector(const QString& list);
void addFieldToActivityInfo(ActivityInfo* info, QString fieldName, ActivityInfoFieldType type);

#endif // UTILITIES_H
