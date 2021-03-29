#pragma once

#include <QtCore>
#include "LuaParserBase.h"

#define PARSERMGR		ParserManager::instance()

namespace spdlog
{
    class logger;
}

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
        QHashIterator<QString, ParserInfo> it(_luaTypes);
        while (it.hasNext())
        {
            parser.insert(it.key(), it.value());
        }
        
        return parser;
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

    std::shared_ptr<spdlog::logger>  _logger;
};

} // namespace owl
