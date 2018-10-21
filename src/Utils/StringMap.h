#pragma once
#include <QtCore>
#include "Exception.h"

namespace owl
{

using StringPairs = std::map<QString, QString>;

class StringMap
{
	StringPairs		_pairs;

public:
	StringMap();
    StringMap(const StringPairs&);
	virtual ~StringMap();

	void parse(const QString& options, const QChar seperator = ' ');
	size_t parseLines(const QString& options);
    
	void add(StringMap* params);
	void add(const QString& key, const char* value);
	void add(const QString& key, const QString& value);
	void add(const QString& key, int val);
	void add(const QString& key, uint val);
	void add(const QString& key, bool val);

	void setOrAdd(const QString& key, const char* value);
	void setOrAdd(const QString& key, const QString& value);
	void setOrAdd(const QString& key, int val);
	void setOrAdd(const QString& key, uint val);
	void setOrAdd(const QString& key, bool val);
    
    bool getBool(const QString& key, bool bThrow = true) const;

    int getInt(const QString& key, int iDefaultVal) const;
    int getInt(const QString& key, bool bThrow = true) const;

    const QString getText(const QString& key, bool bThrow = true) const;

    bool has(const QString& key) const;
	void erase(const QString& key);
	void clear() { _pairs.clear(); }
	const size_t size() { return _pairs.size(); }

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

typedef std::shared_ptr<StringMap> StringMapPtr;

class StringMapException : public OwlException
{

public:
	StringMapException(const QString& msg, const QString& filename = "", int line = 0)
		: OwlException(msg, filename, line)
	{
		// do nothing
	}

	virtual ~StringMapException() throw()
	{
		// do nothing
	}
};

} // namespace

Q_DECLARE_METATYPE(owl::StringMap)
Q_DECLARE_METATYPE(owl::StringMapPtr)
