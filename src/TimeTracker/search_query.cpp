#include "search_query.h"
#include "error_macros.h"

#include <QDebug>

SearchQuery::SearchQuery(QString query, QVector<ActivityInfo *> activityInfos)
{
    m_activityInfos = activityInfos;
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

QVector<QString> SearchQuery::tokenize(QString query)
{
    query.append(QChar(0x03));
    int           textLength = query.length();
    const QChar * textData   = query.constData();
    bool inQuote = false;
    int start = 0;
    int count = 0;
    int quoteStart = 0;

    QVector<QString> tokens;
    for (int i = 0; i < textLength; ++i)
    {
        ushort c = textData[i].unicode();

        if (c == '\"' || (inQuote && c == 0x03))
        {
            if (!inQuote)
            {
                inQuote = true;
                quoteStart = i;
            }
            else
            {
                inQuote = false;
                if (count != 0)
                {
                    if (quoteStart != start)
                    {
                        QString token;
                        token.reserve(count);
                        int numCharsBeforeQuoteStart = quoteStart - start;
                        ERR_VERIFY_V(numCharsBeforeQuoteStart > 0, QVector<QString>());
                        token.append(&query.constData()[start], numCharsBeforeQuoteStart);
                        int numCharsAfterQuoteStart = count - numCharsBeforeQuoteStart;
                        token.append(&query.constData()[quoteStart + 1], numCharsAfterQuoteStart);

                        tokens.append(token);

                        quoteStart = 0;
                        start = i + 1;
                        count = 0;
                        continue;
                    }
                    else
                    {
                        tokens.append(query.mid(start + 1, count));
                        start = i + 1;
                        count = 0;
                        continue;
                    }
                }
            }
        }
        else if (inQuote)
        {
            ++count;
        }
        else if (c == ' ' || c == 0x03)
        {
            if (count != 0)
            {
                tokens.append(query.mid(start, count));
            }
            start = i + 1;
            count = 0;
        }
        else
        {
            ++count;
        }
    }

    return tokens;
}

SearchQuery::ParseResult SearchQuery::parse(QVector<QString> tokens)
{
    ParseResult result;
    for (const QString& token : tokens)
    {
        bool isProperty = false;
        int tokenLength = token.length();

        for (int i = 0; i < tokenLength; ++i)
        {
            ushort c = token[i].unicode();

            if (c == ':')
            {
                if (i == tokenLength - 1)
                {
                    qDebug() << "no property value for" << token;
                    continue;
                }
                if (i == 0)
                {
                    qDebug() << "no property key for" << token;
                    continue;
                }
                QString key = token.mid(0, i);
                QString value = token.mid(i + 1);

                result.properties.append(Property { key, value });
                isProperty = true;
                break;
            }
        }

        if (!isProperty)
        {
            result.searchWords.append(token);
        }
    }

    return result;
}

bool SearchQuery::buildSqlQueryString(SearchQuery::GeneratedSqlQuery * result, QVector<Property> properties, QVector<QString> searchWords)
{
    ERR_VERIFY_NULL_V(result, false);
    result->isValid = false;

    QString query = "SELECT * FROM activity WHERE";
    bool propertiesAdded = false;

    {
        bool bracketsAdded = false;
        bool appendOr = false;
        for (auto property = properties.constBegin(); property != properties.constEnd(); ++property)
        {
            if (property->key == QStringLiteral("category"))
            {
                ActivityInfo * category = findActivityInfoByName(property->value);
                if (category == nullptr)
                {
                    qDebug() << "unknown category" << property->value;
                    continue;
                }

                if (!bracketsAdded)
                {
                    query.append(QStringLiteral(" ("));
                    bracketsAdded = true;
                }

                if (appendOr) query.append(QStringLiteral(" or"));

                query.append(QStringLiteral(" activity_info_id = ?"));
                result->args.append(category->id);

                appendOr = true;
                propertiesAdded = true;
            }
            else
            {
                qDebug() << "unknown property" << property->key;
            }
        }

        if (bracketsAdded)
        {
            query.append(')');
        }
    }

    {
        bool bracketsAdded = false;
        bool appendOr = false;
        for (auto searchWord = searchWords.constBegin(); searchWord != searchWords.constEnd(); ++searchWord)
        {
            if (propertiesAdded && !bracketsAdded)
            {
                query.append(" and (");
                bracketsAdded = true;
            }

            if (appendOr) query.append(" or");

            query.append(" name LIKE ?");
            result->args.append(QStringLiteral("%") + searchWord + QStringLiteral("%"));

            appendOr = true;
        }

        if (bracketsAdded)
        {
            query.append(')');
        }
    }

    qDebug() << "query" << query;
    qDebug() << "args" << result->args;

    result->query   = query;
    result->isValid = true;

    return true;
}

bool SearchQuery::parseQueryString(QString query)
{
    ParseResult parseResult = parse(tokenize(query));

    if (!buildSqlQueryString(&m_sqlQuery, parseResult.properties, parseResult.searchWords))
    {
        m_isValid = false;
        return false;
    }

    m_sourceQuery = query;
    m_isValid = true;
    return true;
}

ActivityInfo * SearchQuery::findActivityInfoByName(QString name)
{
    for (auto info : m_activityInfos)
    {
        if (name.compare(info->name, Qt::CaseInsensitive) == 0)
        {
            return info;
        }
    }

    return nullptr;
}
