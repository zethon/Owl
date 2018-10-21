#pragma once

#include <QtCore>
#include <log4qt/logger.h>
#include "LuaParserBase.h"

#define PARSERMGR		ParserManager::instance()

namespace owl
{

struct ParserInfo
{
	QString name;
	QString url;
	QString filename;
	QString prettyName;
	QMetaObject metaObject;

	ParserInfo()
	{
		// do nothing
	}

	ParserInfo(const QString& pName, const QString& pPrettyName, QMetaObject meta)
		: name(pName),
		  prettyName(pPrettyName),
		  metaObject(meta)
	{
		// nothing to do
	}
};

class ParserBase;

class ParserManager;
using ParserManagerPtr = std::shared_ptr<ParserManager>;
typedef QList<ParserBasePtr> ParserList;

class ParserManager : public QObject
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
    
public:
	static ParserManagerPtr instance();

    ParserManager (const ParserManager&) = delete;
	virtual ~ParserManager(); 
	
	void init(bool bLoadLuaParsers, QString luaParserFolder = QString());

	size_t getParserTypeCount() const 
	{ 
		return _nativeParsers.size() + _luaTypes.size(); 
	}

	ParserBasePtr createParser(const QString& name, const QString& baseUrl, bool bDoThrow = true);

	QList<QString> getParserNames() const;
	QHash<QString, ParserInfo> getParsers()
    {
        auto parser = _nativeParsers;
        return parser.unite(_luaTypes);
    }

protected:
  	ParserManager();

private:
	void loadLuaParsers(QString parserFolder = QString());
	void initLuaParser(const QString& filename, ParserInfo& info);
	QStringList ignoredParserFiles(const QDir& luaPath);

	QHash<QString, ParserInfo> _nativeParsers;
	QHash<QString, ParserInfo> _luaTypes;

	bool _isInitialized;

	static ParserManagerPtr _instance;
};

} // namespace owl
