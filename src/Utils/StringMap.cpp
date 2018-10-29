// Owl - www.owlclient.com
// Copyright (c) 2012-2018, Adalid Claure <aclaure@gmail.com>

#include "StringMap.h"
#include <QMetaType>

namespace owl
{

void StringMap::merge(const StringMap& other)
{
    for (const auto&[key, value] : other)
    {
        add(key, value);
    }
}

void StringMap::parse(const QString& options, const QChar seperator)
{
    QStringList list = options.split(seperator);

    for (auto param : list)
    {
        param = param.trimmed().remove(QRegExp("^\\-"));

        if (param.contains('='))
        {
            QStringList tempList = param.split('=');

            if (tempList.size() > 0)
            {
                if (tempList.size() > 1)
                {
                    add(tempList.at(0), ((QStringList)tempList.mid(1)).join("="));
                }
                else
                {
                    add(tempList.at(0), true);
                }
            }
        }
        else
        {
            add(param, true);
        }
    }
}

size_t StringMap::parseLines(const QString& options, const QChar seperator)
{
    const auto iBeforeSize = _pairs.size();

    QString strTemp(options);
    QTextStream stream(&strTemp);

    while (!stream.atEnd())
    {
        QString line = stream.readLine().simplified();
        if (!line.isEmpty())
        {
            parse(line, seperator);
        }
    }

    return (_pairs.size() - iBeforeSize);
}

bool StringMap::getBool(const QString& key, bool doThrow) const
{
    bool retval = false;
    const auto value = _pairs.find(key);

    if (value != _pairs.end())
    {
        const auto& valstr{ (*value).second };
        retval = (valstr == "1" || valstr.toLower() == "true" || valstr.toLower() == "yes");
    }
    else if (doThrow)
    {
        OWL_THROW_EXCEPTION(owl::StringMapException(
            QString("Could not get boolean value: key '%1' does not exist").arg(key)));
    }

    return retval;
}

QString StringMap::getText(const QString& key, bool bThrow /*= true*/) const
{
    QString strRet;
    const auto i = _pairs.find(key);

    if (i != _pairs.end())
    {
        strRet = (*i).second;
    }
    else if (bThrow)
    {
        OWL_THROW_EXCEPTION(StringMapException(QString("Could notget text value: key '%1' not found").arg(key)));
    }

    return strRet;
}

bool StringMap::has(const QString& key) const
{
    return (_pairs.find(key) != _pairs.end());
}

void StringMap::erase(const QString& key)
{
    const auto i = _pairs.find(key);
    if (i != _pairs.end())
    {
        _pairs.erase(i);
    }
}

QString StringMap::encode() const
{
    QString retStr;

    if (_pairs.size() > 0)
    {
        for (const auto& p : _pairs)
        {
            retStr += QString("%1=%2&").arg(p.first).arg(p.second);
        }
    }

    return retStr;
}

} // namespace
