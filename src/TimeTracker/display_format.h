#ifndef DISPLAY_FORMAT_H
#define DISPLAY_FORMAT_H

#include <QString>
#include <QStringRef>
#include <QStringList>
#include <QVector>


enum DisplayFormatRunType
{
    ConstantString,
    DynamicString
};

struct DisplayFormatRun
{
    DisplayFormatRunType type;
    int dsIndex;
    QStringRef constString;

    DisplayFormatRun();
    ~DisplayFormatRun();
};

class DisplayFormat
{
public:
    DisplayFormat();

    void setFormat(QString format, QStringList names);
    QString format(const QStringList& values);
private:
    QStringList m_names;
    QString m_format;

    QVector<DisplayFormatRun> m_parsedFormat;
};

#endif // DISPLAY_FORMAT_H
