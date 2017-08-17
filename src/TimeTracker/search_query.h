#ifndef SEARCH_QUERY_H
#define SEARCH_QUERY_H

#include <QVector>
#include <QString>
#include <QStringRef>

class SearchQuery
{
public:
    explicit SearchQuery(QString query);

    QString sqlQueryString();
    bool isValid();
private:
    bool parseQueryString(QString query);
    void addTextChunk(int start, int end);
    void parseToken(int start, int end);

    QString m_sourceQuery;

    bool m_isParsed;
    QVector<QStringRef> m_queryTextChunks;

    // Supported properties:
    bool m_hasDateProperty;
    QString m_dateProperty;

};

#endif // SEARCH_QUERY_H
