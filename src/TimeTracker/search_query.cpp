#include "search_query.h"
#include "error_macros.h"

SearchQuery::SearchQuery(QString query)
{
    parseQueryString(query);
}

QString SearchQuery::sqlQueryString()
{
    return "SELECT * FROM activities"; // @TODO: FIX ME
}

bool SearchQuery::isValid()
{
    return m_isParsed;
}

bool SearchQuery::parseQueryString(QString query)
{
    if (query.isEmpty()) return false;
    m_sourceQuery = query;

    addTextChunk(0, query.length() - 1);

//    enum class ParseState
//    {
//        Chunk,
//        Property,
//    } parseState = ParseState::Chunk;

//    int start = -1;
//    int end   = 0;

//    int textLength = query.length();

//    const QChar * textData = query.constData();
//    ERR_VERIFY_NULL_V(textData, false);

//    QStringRef propertyKey;
//    QStringRef propertyValue;

//    enum class QuoteType
//    {
//        None,
//        Single,
//        Double
//    } quoteType = QuoteType::None;
//    QChar c;
//    for (int i = 0; i < textLength; ++i)
//    {
//        c = textData[i];

//        if (c == QChar(' '))
//        {
//            if (start == -1) continue;

//        }

////        if (parseState == ParseState::Chunk)
////        {
////            if (start == -1)
////            {
////                start = i;
////                end = 0;
////                continue;
////            }
////            else
////            {
////                if (c == QChar(' '))
////                {
////                    addTextChunk(start, end);
////                    start = -1;
////                    end = 0;
////                }
////                else if (c == QChar(':') && )
////                {
////                    parseState = ParseState::Property;
////                    propertyKey =
////                }
////                else
////                {
////                    ++end;
////                }
////                continue;
////            }
////        }
//    }

    return true;
}

void SearchQuery::addTextChunk(int start, int end)
{
    ERR_VERIFY(start <= end);
    ERR_VERIFY(start >= 0);
    ERR_VERIFY(end >= 0);

    QStringRef chunk(&m_sourceQuery, start, end - start);
    m_queryTextChunks.append(chunk);
}

