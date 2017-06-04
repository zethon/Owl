// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#include "StringMap.h"
#include <QMetaType>

namespace owl
{

StringMap::StringMap()
{
	// do nothing
}
    
StringMap::StringMap(const StringPairs& pairs)
{
    for (const auto& p : pairs)
    {
        add(p.first, p.second);
    }
}

StringMap::~StringMap()
{
	// do nothing
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
    
size_t StringMap::parseLines(const QString& options)
{
    const auto iBeforeSize = _pairs.size();
    
    QString strTemp(options);
    QTextStream stream(&strTemp);
    
    while (!stream.atEnd())
    {
        QString line = stream.readLine().simplified();
		if (!line.isEmpty())
		{
			parse(line);
		}
    }
    
    return (_pairs.size() - iBeforeSize);
}

void StringMap::add(StringMap* params)
{
    for (const auto& p : *params)
    {
        add(p.first, p.second);
    }
}

void StringMap::add(const QString& key, const QString& value)
{
    _pairs.insert(std::make_pair(key, value));
}

void StringMap::add(const QString& key, const char* value)
{
    _pairs.insert(std::make_pair(key, QString(value)));
}

void StringMap::add(const QString& key, int val)
{
	add(key, QString::number(val));
}

void StringMap::add(const QString& key, uint val)
{
    add(key, QString::number(val));
}
    
void StringMap::add(const QString& key, bool val)
{
	add(key, val ? 1 : 0);
}

void StringMap::setOrAdd(const QString& key, const char* value)
{
	if (has(key))
	{
		erase(key);
	}

	add(key, value);
}

void StringMap::setOrAdd(const QString& key, const QString& value)
{
	if (has(key))
	{
		erase(key);
	}

	add(key, value);
}

void StringMap::setOrAdd(const QString& key, int value)
{
	if (has(key))
	{
		erase(key);
	}

	add(key, value);
}
    
void StringMap::setOrAdd(const QString& key, uint value)
{
    if (has(key))
    {
        erase(key);
    }
    
    add(key, value);
}

void StringMap::setOrAdd(const QString& key, bool value)
{
	if (has(key))
	{
		erase(key);
	}

	add(key, value);
}

bool StringMap::getBool(const QString& key, bool bThrow /*= true*/) const
{
	bool bRet = false;
    const auto value = _pairs.find(key);

    if (value != _pairs.end())
	{
        const QString valstr { (*value).second };
        bRet = (valstr == "1" || valstr.toLower() == "true" || valstr.toLower() == "yes");
	}
	else if (bThrow)
	{
        OWL_THROW_EXCEPTION(StringMapException(QString("Could not getBool(), key '%1' not found").arg(key)));
	}
   
    return bRet;
}

int	StringMap::getInt(const QString& key, int iDefaultVal) const
{
	int iRet = 0;
    const auto i = _pairs.find(key);

	if (i != _pairs.end())
	{
		bool bOk = false;

        iRet = (*i).second.toInt(&bOk);

		if (!bOk)
		{
			iRet = iDefaultVal;
		}
	}
	else
	{
		iRet = iDefaultVal;
	}

	return iRet;
}

int StringMap::getInt(const QString& key, bool bThrow /*= true*/) const
{
	int iRet = 0;
    const auto i = _pairs.find(key);

	if (i != _pairs.end())
	{
		bool bOk = false;

        iRet = (*i).second.toInt(&bOk);

		if (!bOk && bThrow)
		{
            OWL_THROW_EXCEPTION(StringMapException(QString("Could not convert val to int in key '%1'").arg(key)));
		}
	}
	else if (bThrow)
	{
        OWL_THROW_EXCEPTION(StringMapException(QString("Could not getInt(), key '%1' not found").arg(key)));
	}

	return iRet;
}

const QString StringMap::getText(const QString& key, bool bThrow /*= true*/) const
{
	QString strRet;
    const auto i = _pairs.find(key);

	if (i != _pairs.end())
	{
        strRet = (*i).second;
	}
	else if (bThrow)
	{
        OWL_THROW_EXCEPTION(StringMapException(QString("Could not getValue(), key '%1' not found").arg(key)));
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
