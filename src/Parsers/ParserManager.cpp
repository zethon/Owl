#include "../Utils/Exception.h"
#include "../Utils/OwlUtils.h"
#include "ParserManager.h"

#include "Tapatalk.h"
#include "Xenforo.h"

#include <spdlog/spdlog.h>

namespace owl
{

ParserManagerPtr ParserManager::instance()
{
	if (!_instance.get())
	{
		_instance.reset(new ParserManager());
	}

	return _instance;
}

ParserManager::ParserManager()
    : _isInitialized(false),
      _logger { spdlog::get("Owl")->clone("ParserManager") }
{
    spdlog::register_logger(_logger);
}	
	
ParserManager::~ParserManager()
{
	// do nothing
}

void ParserManager::init(bool bLoadLuaParsers, QString luaParserFolder)
{
	if (_isInitialized)
	{
		OWL_THROW_EXCEPTION(owl::OwlException("ParserManager has already been initialized."));
	}

    _nativeParsers.insert(TAPATALK_NAME, ParserInfo(TAPATALK_NAME, TAPATALK_PRETTYNAME, Tapatalk4x::staticMetaObject));
    _nativeParsers.insert(XENFORO_NAME, ParserInfo(XENFORO_NAME, XENFORO_PRETTYNAME, Xenforo::staticMetaObject));

	if (bLoadLuaParsers)
	{
		loadLuaParsers(luaParserFolder);
	}
    else
    {
        _logger->debug("Lua Parsers disabled");
    }

	_isInitialized = true;

    _logger->info("{} parser(s) loaded", (int)getParserTypeCount());
}

owl::ParserBasePtr ParserManager::createParser(const QString& name, const QString& baseUrl, bool bDoThrow /*= true*/ )
{
	ParserBasePtr ret;

	if (_nativeParsers.contains(name))
	{
		QMetaObject metaObj = _nativeParsers.value(name).metaObject;
        ParserBase* base = (ParserBase*)metaObj.newInstance(Q_ARG(const QString&, baseUrl));

		ret = ParserBasePtr(base);
	}
	else if (_luaTypes.contains(name))
	{
		ParserInfo info = _luaTypes.value(name);
		ret = LuaParserBasePtr(new LuaParserBase(baseUrl, info.filename));
	}
	else if (bDoThrow)
	{
		QString error = QString("CreateParser failed. Unknown parser type '%1'").arg(name);
        _logger->warn(error.toStdString());

        OWL_THROW_EXCEPTION(OwlException(error));
	}

	return ret;
}

void ParserManager::loadLuaParsers(QString parserFolder)
{
	QString pathName = QDir(QDir::currentPath()).filePath("parsers");

	if (!parserFolder.isEmpty())
	{
		pathName = parserFolder;
	}

	QString luaPath = QString("%1;%2\\?.lua;%2\\include\\?.lua")
		.arg(QString(qgetenv("LUA_PATH")))
		.arg(parserFolder);

	qputenv("LUA_PATH",QByteArray(luaPath.toLatin1()));

	QDir parserDir(pathName);
	parserDir.setSorting(QDir::Name);

    _logger->debug("Loading parsers in folder '{}'", parserFolder.toStdString());

	auto ignoredParsers = ignoredParserFiles(parserDir);
    
    QStringList filters;
    filters << "*.lua" << "*.owl" << "*.parser";
    parserDir.setNameFilters(filters);
    
	ParserInfo parserInfo;
    for (QFileInfo info : parserDir.entryInfoList())
	{
		if (ignoredParsers.contains(info.fileName(), Qt::CaseInsensitive))
		{
            _logger->trace("Skipping parser file '{}' on ignore list", info.fileName().toStdString());
			continue;
		}

		QString filePath(info.absoluteFilePath());
        _logger->trace("Loading parser file '{}'", filePath.toStdString());

		initLuaParser(filePath, parserInfo);
		if (!parserInfo.name.isEmpty())
		{
			if (ignoredParsers.contains(parserInfo.name, Qt::CaseInsensitive))
			{
                _logger->trace("Skipping parser named '{}' on ignore list", info.fileName().toStdString());
				continue;
			}

			_luaTypes.insert(parserInfo.name, parserInfo);
            _logger->info("Sucesfully Loaded parser name `{}`", parserInfo.name.toStdString());
		}
		else
		{
            _logger->error("Failed to load parser `{}`", filePath.toStdString());
		}
	}
}

void ParserManager::initLuaParser(const QString& filename, ParserInfo& info)
{
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	int luaStatus = luaL_loadfile(L, filename.toLatin1());

	if (!luaStatus && !lua_pcall(L, 0, 0, 0))
	{
		lua_getglobal(L, "parserName");
		if (lua_isstring(L, -1))
		{
			info.name = QString(lua_tostring(L, -1));
		}
		else
		{
            _logger->warn("Invalid parser file ({}): 'parserName' should be a string", filename.toStdString());
		}
        
        if (_luaTypes.contains(info.name))
        {
            auto otherInfo = _luaTypes[info.name];
            
            _logger->warn("Parser with name '{}' already loaded from '{}'. Not loading from file '{}'",
                info.name.toStdString(), otherInfo.filename.toStdString(), filename.toStdString());
            info.name.clear();
        }
        else
        {
            lua_getglobal(L, "parserPrettyName");
            if (lua_isstring(L, -1))
            {
                info.prettyName = QString(lua_tostring(L, -1));
            }
            else
            {
                info.prettyName = info.name;
            }

            lua_getglobal(L, "boardUrl");
            if (lua_isstring(L, -1))
            {
                info.url = QString(lua_tostring(L, -1));
            }

            info.filename = filename;
        }
	}
	else
	{
		QString strError(lua_tostring(L, -1));
        _logger->error("Invalid parser file ({}): {}", filename.toStdString(), strError.toStdString());
	}

	lua_close(L);
}

QStringList ParserManager::ignoredParserFiles(const QDir& luaPath)
{
	QStringList retList;
	QFile file(luaPath.filePath(".owlignore"));

	if (file.exists() && file.open(QIODevice::ReadOnly))
	{
        _logger->debug("Using parser ignore file at: {}", file.fileName().toStdString());

		QTextStream in(&file);

		while (!in.atEnd())
		{
			QString line = in.readLine().trimmed();

			if (!line.isEmpty() && line[0] != ';')
			{
				retList.push_back(line);
			}
		}
	}
	
	return retList;
}


QList<QString> ParserManager::getParserNames() const
{
	QList<QString> retlist;

	QHashIterator<QString, ParserInfo> it(_nativeParsers);
	while (it.hasNext())
	{
		it.next();
		retlist.push_back(it.key());
	}

	QHashIterator<QString, ParserInfo> lit(_luaTypes);
	while (lit.hasNext())
	{
		lit.next();
		retlist.push_back(lit.key());
	}

	return retlist;
}

ParserManagerPtr ParserManager::_instance;

} // namespace owl
