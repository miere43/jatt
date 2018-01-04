#include "search_query.h"
#include "error_macros.h"

#include <QDebug>

SearchQuery::SearchQuery(QString query, QVector<ActivityCategory *> categories)
{
    m_activityCategories = categories;
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

void SearchQuery::beginGroup()
{
    ERR_VERIFY(!m_hasGroup);
    m_hasGroup = true;
    m_groupItems = 0;
    m_groupTempText.clear();
}

void SearchQuery::appendSql(QString seperator, QString text)
{
    ERR_VERIFY(m_hasGroup);
    if (m_groupItems == 0)
    {
        if (m_groups != 0) m_groupTempText.append(" and");
        m_groupTempText.append(" (");
    }
    else if (m_groupItems > 0)
    {
        m_groupTempText.append(seperator);
    }
    m_groupTempText.append(text);
    ++m_groupItems;
}

void SearchQuery::endGroup()
{
    ERR_VERIFY(m_hasGroup);
    if (m_groupItems > 0)
    {
        m_groupTempText.append(")");
        m_sqlQuery.query.append(m_groupTempText);
        m_groups += 1;
    }
    m_hasGroup = false;
}

bool SearchQuery::buildSqlQueryString(SearchQuery::GeneratedSqlQuery * result, QVector<Property> properties, QVector<QString> searchWords)
{
    ERR_VERIFY_NULL_V(result, false);
    result->isValid = false;
    result->query = QStringLiteral("SELECT * FROM activity WHERE");
    QString & query = result->query;

    QVector<ActivityCategory *> categories;
    QVector<QString> names;

    for (auto property = properties.constBegin(); property != properties.constEnd(); ++property)
    {
        if (property->key == QStringLiteral("category"))
        {
            ActivityCategory * category = findActivityCategoryByName(property->value);
            if (category == nullptr)
            {
                qDebug() << "unknown category" << property->value;
                continue;
            }

            categories.append(category);
        }
        else if (property->key == QStringLiteral("name"))
        {
            names.append(property->value);
            // select * from activity where (activity_info_id = 1 or activity_info_id = 2) and (name = "hello")
        }
        else
        {
            qDebug() << "unknown property" << property->key;
        }
    }

    if (categories.count() > 0)
    {
        beginGroup();
        for (const ActivityCategory * category : categories)
        {
            appendSql(" and", " activity_info_id = ?");
            m_sqlQuery.args.append(category->id);
        }
        endGroup();
    }

    if (names.count() > 0)
    {
        beginGroup();
        for (const QString & name : names)
        {
            appendSql(" or", " name = ?");
            m_sqlQuery.args.append(name);
        }
        endGroup();
    }

    if (searchWords.count() > 0)
    {
        beginGroup();
        for (auto searchWord = searchWords.constBegin(); searchWord != searchWords.constEnd(); ++searchWord)
        {
            appendSql(" or", " name LIKE ?"); // @TODO: 'or note LIKE ?'
            result->args.append(QStringLiteral("%") + searchWord + QStringLiteral("%"));
        }
        endGroup();
    }

    qDebug() << "query" << result->query;
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

ActivityCategory * SearchQuery::findActivityCategoryByName(QString name)
{
    for (auto category : m_activityCategories)
    {
        if (name.compare(category->name, Qt::CaseInsensitive) == 0)
        {
            return category;
        }
    }

    return nullptr;
}
