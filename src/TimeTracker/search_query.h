#ifndef SEARCH_QUERY_H
#define SEARCH_QUERY_H

#include <QVector>
#include <QString>
#include <QStringRef>
#include <QVariant>

class SearchQuery
{
public:
    struct GeneratedSqlQuery
    {
        bool isValid = false;
        QString query;
        QVector<QVariant> args;
    };

    explicit SearchQuery(QString query);

    GeneratedSqlQuery sqlQuery();
    bool isValid();
private:
    bool parseQueryString(QString query);
    void addTextChunk(int start, int end);
    void parseToken(int start, int end);
    bool buildSqlQueryString(GeneratedSqlQuery * result, QVector<QStringRef> tokens);
    QVector<QStringRef> tokenize(QString query);

    QString m_sourceQuery;

    bool m_isValid = false;
    GeneratedSqlQuery m_sqlQuery;

};

#endif // SEARCH_QUERY_H
