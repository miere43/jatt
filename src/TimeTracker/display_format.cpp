#include "display_format.h"

DisplayFormat::DisplayFormat()
{

}

void DisplayFormat::setFormat(QString format, QStringList names)
{
    m_format = format;
    m_names = names;
    m_parsedFormat.clear();

    int csStart = -1;
    int i = 0;
    for (; i < m_format.length(); ++i)
    {
        QChar c = m_format[i];

        bool dynvalSuccess = false;
        int jumpOffset = 0;
        int nameIndex = -1;
        if (c == '{' && i + 2 < m_format.length() && m_format[i + 1] == '{')
        {
            QString dynamicValue;
            for (int j = i + 2; j < m_format.length(); ++j)
            {
                c = m_format[j];
                if (c != '}' && j + 1 < m_format.length() && m_format[j + 1] == '}')
                {
                    dynamicValue = dynamicValue.trimmed();
                    for (int k = 0; k < m_names.length(); ++k)
                    {
                        if (m_names[k].compare(dynamicValue) == 0)
                        {
                            nameIndex = k;
                            break;
                        }
                    }

                    if (nameIndex == -1)
                    {
                        jumpOffset = 0;
                        dynvalSuccess = false;
                        // unknown dynamic value which is not present in 'names'
                    }
                    else
                    {
                        // (j + 2) - add ending }} brackets.
                        jumpOffset = (j + 2) - i;
                        dynvalSuccess = true;
                    }
                    break;
                }
                else
                {
                    dynamicValue.append(c);
                }
            }

            if (dynvalSuccess)
            {
                if (csStart != -1)
                {
                    DisplayFormatRun run;
                    run.type = ConstantString;
                    run.constString = QStringRef(&m_format, csStart, i - csStart);
                    csStart = -1;
                    m_parsedFormat.append(run);
                }

                DisplayFormatRun run;
                run.type = DynamicString;
                run.dsIndex = nameIndex;
                m_parsedFormat.append(run);
            }
            else
            {
                csStart = (i);
            }

            i += jumpOffset;
            continue;
        }
        else
        {
            if (csStart == -1)
            {
                csStart = i;
                continue;
            }
        }
    }

    if (csStart != -1)
    {
        DisplayFormatRun run;
        run.type = ConstantString;
        run.constString = QStringRef(&m_format, csStart, i - csStart);
        csStart = -1;
        m_parsedFormat.append(run);
    }

    if (m_parsedFormat.isEmpty())
    {
        DisplayFormatRun run;
        if (m_names.isEmpty())
        {
            run.type = ConstantString;
            run.constString = QStringRef(&m_format);
        }
        else
        {
            run.type = DynamicString;
            run.dsIndex = 0;
        }
        m_parsedFormat.append(run);
    }
}

QString DisplayFormat::format(const QStringList &values)
{
    Q_ASSERT(values.count() == m_names.count());

    QString result;
    for (const DisplayFormatRun& run : m_parsedFormat)
    {
        if (run.type == ConstantString)
        {
            result.append(run.constString);
        }
        else if (run.type == DynamicString)
        {
            result.append(values[run.dsIndex]);
        }
        else
        {
            Q_ASSERT(false);
        }
    }

    return result;
}

DisplayFormatRun::DisplayFormatRun()
{

}

DisplayFormatRun::~DisplayFormatRun()
{

}
