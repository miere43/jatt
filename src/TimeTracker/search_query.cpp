#include "search_query.h"
#include "error_macros.h"

#include <QDebug>

SearchQuery::SearchQuery(QString query)
{
    parseQueryString(query);
}

SearchQuery::GeneratedSqlQuery SearchQuery::sqlQuery()
{
    return m_sqlQuery;
}

bool SearchQuery::isValid()
{
    return m_isValid;
}

QVector<QStringRef> SearchQuery::tokenize(QString query)
{
    int  textLength = query.length();
    const QChar * textData = query.constData();
    bool inQuote = false;
    int start = 0;
    int count = 0;
    QVector<QStringRef> tokens;

    for (int i = 0; i < textLength; ++i)
    {
        ushort c = textData[i].unicode();

        if (c == '\"')
        {
            if (inQuote)
            {
                if (count != 0) tokens.append(QStringRef(&query, start, count));
                count = 0;
                start = i + 1;
                inQuote = false;
                continue;
            }
            else
            {
                inQuote = true;
                start = i + 1;
                count = 0;
                continue;
            }
        }
        else if (inQuote)
        {
            ++count;
            continue;
        }
        else if (c == ' ')
        {
            if (count != 0) tokens.append(QStringRef(&query, start, count));
            count = 0;
            start = i + 1;
            continue;
        }
        else
        {
            ++count;
            continue;
        }
    }

    if (count != 0) tokens.append(QStringRef(&query, start, count));
    // @TODO: handle unclosed quote.

    return tokens;
}

bool SearchQuery::buildSqlQueryString(SearchQuery::GeneratedSqlQuery * result, QVector<QStringRef> tokens)
{
    ERR_VERIFY_NULL_V(result, false);
    result->isValid = false;

    QString query = "SELECT * FROM activity WHERE";
    bool first = true;
    for (const QStringRef& token : tokens)
    {
        if (!first)
        {
            query.append(QStringLiteral(" or"));
        }
        query.append(QStringLiteral(" name LIKE ?"));
        result->args.append(QStringLiteral("%") + token.toString() + QStringLiteral("%"));
        first = false;
    }

    qDebug() << "query" << query;
    qDebug() << "args" << result->args;

    result->query   = query;
    result->isValid = true;
    return result;
}

bool SearchQuery::parseQueryString(QString query)
{
    QVector<QStringRef> tokens = tokenize(query);
    if (tokens.count() == 0)
    {
        m_isValid = false;
        return false;
    }

    if (!buildSqlQueryString(&m_sqlQuery, tokens))
    {
        m_isValid = false;
        return false;
    }

    m_sourceQuery = query;
    m_isValid = true;
    return true;
}
