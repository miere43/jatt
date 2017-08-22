#ifndef SEARCH_QUERY_H
#define SEARCH_QUERY_H

#include <QVector>
#include <QString>
#include <QVariant>

#include "core_types.h"

class SearchQuery
{
public:
    struct GeneratedSqlQuery
    {
        bool isValid = false;
        QString query;
        QVector<QVariant> args;
    };

    struct Property
    {
        QString key;
        QString value;
    };

    struct ParseResult
    {
        QVector<Property> properties;
        QVector<QString> searchWords;
    };

    explicit SearchQuery(QString query, QVector<ActivityCategory *> categories);

    GeneratedSqlQuery sqlQuery();
    bool isValid();
private:
    bool parseQueryString(QString query);
    bool buildSqlQueryString(GeneratedSqlQuery * result, QVector<Property> properties, QVector<QString> searchWords);
    QVector<QString> tokenize(QString query);
    ParseResult parse(QVector<QString> tokens);

    // Search activity category by name (case insensitive) in 'm_activityCategories' array.
    ActivityCategory * findActivityCategoryByName(QString name);

    QString m_sourceQuery;

    bool m_isValid = false;
    GeneratedSqlQuery m_sqlQuery;

    QVector<ActivityCategory *> m_activityCategories;
};

#endif // SEARCH_QUERY_H
