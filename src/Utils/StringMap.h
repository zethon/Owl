// Owl - www.owlclient.com
// Copyright (c) 2012-2019, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <QtCore>
#include "Exception.h"

namespace owl
{

class StringMap;
using StringMapPtr = std::shared_ptr<StringMap>;

class StringMapException : public Exception
{

public:
    virtual ~StringMapException() = default;
    using Exception::Exception;
};

class StringMap
{

public:
    using StringPairs = std::map<QString, QString>;

private:
    StringPairs		_pairs;

public:

    StringMap() = default;
    StringMap(const StringMap&) = default;
    StringMap(StringMap&& other) = default;

    StringMap& operator=(const StringMap& other)
    {
        if (&other == this)
        {
            return *this;
        }

        _pairs = other._pairs;
        return *this;
    }

    virtual ~StringMap() = default;

	void parse(const QString& options, const QChar seperator = ' ');
    size_t parseLines(const QString& options, const QChar seperator = ' ');

    void merge(const StringMap& other);
    
    template <typename T> 
    typename std::enable_if<(
            std::is_integral<T>::value && !std::is_same<T, bool>::value)>::type
    add(const QString& key, T val)
    {
        _pairs.insert(std::make_pair(key, QString::number(val)));
    }

	void add(const QString& key, bool val)
    {
	    add(key, val ? 1 : 0);
    }

    void add(const QString& key, const char* val)
    {
        _pairs.insert(std::make_pair(key, 
            val ? QString::fromUtf8(val) : QString{}));
    }

    void add(const QString& key, const QString& val)
    {
        _pairs.insert(std::make_pair(key, val));
    }

    void setOrAdd(const QString& key, const char* value)
    {
        _pairs.insert_or_assign(key,
            value ? QString::fromUtf8(value) : QString{});
    }

    void setOrAdd(const QString& key, const QString& value)
    {
        _pairs.insert_or_assign(key, value);
    }

    template <typename T>
    typename std::enable_if<(
        std::is_integral<T>::value && !std::is_same<T, bool>::value)>::type
    setOrAdd(const QString& key, T val)
    {
        _pairs.insert_or_assign(key, QString::number(val));
    }

    void setOrAdd(const QString& key, bool val)
    {
        setOrAdd(key, val ? 1 : 0);
    }

    bool getBool(const QString& key, bool bThrow = true) const;

    template<typename T>
    typename std::enable_if<std::is_integral<T>::value, T>::type
    get(const QString& key, bool doThrow = true) const
    {
        T retval = 0;
        auto i = _pairs.find(key);

        if (i != _pairs.end())
        {
            bool bOk = false;
            retval = i->second.toInt(&bOk);

            if (!bOk && doThrow)
            {
                OWL_THROW_EXCEPTION(owl::StringMapException(
                    QString("Could not get integral value: key '%1' has invalid integral value").arg(key)));
            }
        }
        else if (doThrow)
        {
            OWL_THROW_EXCEPTION(owl::StringMapException(
                QString("Could not get integral value: key '%1' does not exist").arg(key)));
        }

        return retval;
    }

    QString getText(const QString& key, bool bThrow = true) const;

    bool has(const QString& key) const;
	void erase(const QString& key);
	void clear() { _pairs.clear(); }
	size_t size() const { return _pairs.size(); }

	QString encode() const;
    
	auto begin() const -> decltype(_pairs.begin())
	{
		return _pairs.begin();
	}
	
	auto end() const -> decltype(_pairs.end())
	{
		return _pairs.end();
	}
};

} // namespace

std::ostream & operator<<(std::ostream& os, const owl::StringMap& params);

Q_DECLARE_METATYPE(owl::StringMap)
Q_DECLARE_METATYPE(owl::StringMapPtr)
