#include "../Utils/OwlUtils.h"
#include "OwlLua.h"
#include "LuaParserBase.h"

#include <spdlog/spdlog.h>

namespace owl
{

LuaParserException::LuaParserException(StringMap params, const QString& filename, const int iLine)
	: OwlException(params.getText("error-text", false), filename, iLine),
	  _params(params)
{
}

LuaParserBase::LuaParserBase(const QString& url, const QString& luaFile)
	: ParserBase("#luaparser", "#luaparser", url),
	  _strLuaFile(luaFile),
	  _stateObjIdx(0),
      L(luaL_newstate()),
      _logger { spdlog::get("Owl")->clone("LuaParserBase") }
{
    spdlog::register_logger(_logger);

    _stateMutex = std::make_shared<std::mutex>();
	luaL_openlibs(L);

	// store a reference to 'this' as a global variable so we
	// can reference it later
	lua_newtable(L); 
	lua_pushlightuserdata(L,(void*)this);
	lua_setglobal(L, "__parserObj");

	// register Owl interface in the Lua scripts
	registerFunctions();

	int luaStatus = luaL_loadfile(L, _strLuaFile.toLatin1());

	if (luaStatus || lua_pcall(L, 0, 0, 0))
	{
		QString strMsg = QString("could not initialize lua parser '%1': %2")
			.arg(luaFile)
			.arg(lua_tostring(L, -1));
        _logger->error(strMsg.toStdString());

        OWL_THROW_EXCEPTION(LuaException(strMsg));
	}

	lua_getglobal(L, "Parser");
	lua_getfield(L, -1, "create");
	lua_pushstring(L, this->getBaseUrl().toLatin1());

	if (lua_pcall(L, 1, 1, 0) != 0)
	{
		QString strMsg = QString("problem calling 'createParser' in %1: %2")
			.arg(luaFile)
			.arg(lua_tostring(L, -1));
        _logger->error(strMsg.toStdString());

        OWL_THROW_EXCEPTION(LuaException(strMsg));
	}

	int top = lua_gettop(L);

	lua_getglobal(L,"boardware");
	_options->add("boardware",luaL_checkstring(L, -1));

	lua_getglobal(L,"boardwaremax");
	_options->add("boardwaremax",luaL_checkstring(L, -1));

	lua_getglobal(L,"boardwaremin");
	_options->add("boardwaremin",luaL_checkstring(L, -1));

	// Reset stack
	lua_settop(L, top);	

	// store the lua state
	_stateObjIdx = luaL_ref(L, LUA_REGISTRYINDEX);
}

LuaParserBase::~LuaParserBase()
{   
    // HACK: the Lua state needs to be closed when prudent!
    if (_doCloseState)
    {
        lua_close(L);
    }
}

void LuaParserBase::registerFunctions()
{
	// register the webclient object
	luaL_newmetatable(L, "Owl.webclient");
	lua_pushliteral(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);						// metatable.__index = Owl.webclient
	lua_pushliteral(L, "__call");			// __call(c)
	luaL_getmetatable(L, "Owl.webclient");
	luaL_setfuncs(L, webclientlib, 0);
	lua_setglobal(L, "webclient");			// global object (ie. `local w = webclient.new()`)

	// register the sgmldoc object
	luaL_newmetatable(L, "Owl.sgml");
	lua_pushliteral(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);						// metatable.__index = Owl.sgmldoc
	lua_pushliteral(L, "__call");			// __call(c)
	luaL_getmetatable(L, "Owl.sgml");
	luaL_setfuncs(L, sgmllib, 0);
	lua_setglobal(L, "sgml");				// global object (ie. `local w = sgml.new()`)

	// register the regexp object
	luaL_newmetatable(L, "Owl.regexp");
	lua_pushliteral(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);						// metatable.__index = Owl.sgmldoc
	lua_pushliteral(L, "__call");			// __call(c)
	luaL_getmetatable(L, "Owl.regexp");
	luaL_setfuncs(L, regexplib, 0);
	lua_setglobal(L, "regexp");				// global object (ie. `local w = sgml.new()`)

	// register the sgmltag object
	luaL_newmetatable(L, "Owl.sgmltag");
	lua_pushliteral(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);						// metatable.__index = Owl.sgmldoc
	lua_pushliteral(L, "__call");			// __call(c)
	luaL_getmetatable(L, "Owl.sgmltag");
	luaL_setfuncs(L, sgmltaglib, 0);
	lua_setglobal(L, "sgmltag");			// global object (ie. `local w = sgml.new()`)

	lua_newtable(L);
	luaL_setfuncs(L, owlutilslib, 0);
	lua_setglobal(L, "utils");				// utils object

	lua_newtable(L);
	luaL_setfuncs(L, errorlib, 0);
	lua_setglobal(L, "error");

	lua_newtable(L);
	lua_pushstring(L, "FORUM");
	lua_pushnumber(L, Forum::FORUM);
	lua_settable(L, -3);
	lua_pushstring(L, "CATEGORY");
	lua_pushnumber(L, Forum::CATEGORY);
	lua_settable(L, -3);
	lua_pushstring(L, "LINK");
	lua_pushnumber(L, Forum::LINK);
	lua_settable(L, -3);
	lua_setglobal(L, "ForumType");
}

QString LuaParserBase::getName() const
{
    std::lock_guard<std::mutex> locker(*_stateMutex);

	QString retval;
	lua_getglobal(L, "parserName");

	if (lua_isstring(L, -1))
	{
		retval = QString(lua_tostring(L, -1));
	}

	return retval;
}

QString LuaParserBase::getPrettyName() const
{
    std::lock_guard<std::mutex> locker(*_stateMutex);

	QString retval;
    lua_getglobal(L, "parserName");

	if (lua_isstring(L, -1))
	{
		retval = QString(lua_tostring(L, -1));
	}

	return retval;
}

QString LuaParserBase::getItemUrl(ForumPtr forum)
{
	return getItemUrlHelper("getForumUrl", forum->getId());
}

QString LuaParserBase::getItemUrl(ThreadPtr thread)
{
	return getItemUrlHelper("getThreadUrl", thread->getId());
}

QString LuaParserBase::getItemUrl(PostPtr post)
{
	return getItemUrlHelper("getPostUrl", post->getId());
}

QString LuaParserBase::getItemUrlHelper(const QString& funcName, const QString itemId)
{
    std::lock_guard<std::mutex> locker(*_stateMutex);

	// clear the stack
	lua_settop(L, 0);

	// load the method from the class definition
	lua_getglobal(L, "Parser");
	lua_getfield(L, -1, funcName.toLatin1());

	// pass the reference to the created object as the 1st param
	lua_rawgeti(L, LUA_REGISTRYINDEX, _stateObjIdx);

    // push the forumId
    lua_pushstring(L, itemId.toLatin1());

	if (lua_pcall(L, 2, 1, 0) != 0)
	{
        _logger->error("Lua call to {} failed: {}", funcName.toStdString(), lua_tostring(L, -1));
        OWL_THROW_EXCEPTION(LuaException(lua_tostring(L, -1)));
	}

	QString url;
	if (lua_isstring(L, -1))
	{
		url = lua_tostring(L, -1);
	}

	return url;
}

QString LuaParserBase::getPostQuote(PostPtr post)
{
    std::lock_guard<std::mutex> locker(*_stateMutex);

	// clear the stack
	lua_settop(L, 0);

	// load the method from the class definition
	lua_getglobal(L, "Parser");
	lua_getfield(L, -1, "getPostQuote");

	// pass the reference to the create object as the 1st param
	lua_rawgeti(L, LUA_REGISTRYINDEX, _stateObjIdx);
	
	// pass a login table as a param
	lua_newtable(L);
	lua_pushstring(L, post->getId().toLatin1());
	lua_setfield(L, -2, "postId");
	lua_pushstring(L, post->getAuthor().toLatin1());
	lua_setfield(L, -2, "postAuthor");
	lua_pushstring(L, post->getText().toLatin1());
	lua_setfield(L, -2, "postText");

	QString quote;
	if (lua_pcall(L, 2, 1, 0) == 0 && lua_isstring(L, -1))
	{
		quote = lua_tostring(L, -1);
	}
	else
	{
		QString strMsg = QString("problem calling 'getPostQuotein %1: %2")
			.arg(_strLuaFile)
			.arg(lua_tostring(L, -1));
        _logger->warn(strMsg.toStdString());

		quote = ParserBase::getPostQuote(post);
	}

	return quote;
}

ParserBasePtr LuaParserBase::clone(ParserBasePtr other)
{
    // NOTE: There is no need to set the WebClient's CookieJar since
    // a clone will use the *SAME* Lua-state as the original object
    // and therefor the same WebClient object (and cookie jar)
    LuaParserBasePtr retval = std::make_shared<LuaParserBase>(getBaseUrl(), _strLuaFile);
    ParserBase::clone(retval);

    retval->_dtParser = _dtParser;
    retval->_stateObjIdx = _stateObjIdx;
    retval->L = L;

    // make sure we don't close the singleton Lua state when this object is destroryed
    retval->_doCloseState = false;

    // delete the old LuaState mutex and assign ours
    retval->_stateMutex.reset();
    retval->_stateMutex = _stateMutex;

    return retval;
}

QString LuaParserBase::getLastRequestUrl()
{
    // clear the stack
    lua_settop(L, 0);

    // load the method from the class definition
    lua_getglobal(L, "Parser");
    lua_getfield(L, -1, "getLastRequestUrl");

    // pass the reference to the create object as the 1st param
    lua_rawgeti(L, LUA_REGISTRYINDEX, _stateObjIdx);

    if (lua_pcall(L, 1, 1, 0) != 0)
    {
        _logger->error("Lua call to getLastRequestUrl failed: {}", lua_tostring(L, -1));
        OWL_THROW_EXCEPTION(LuaException(lua_tostring(L, -1)));
    }

    return QString(lua_tostring(L, -1));
}

QVariant LuaParserBase::doLogin(const LoginInfo& info)
{
    std::lock_guard<std::mutex> locker(*_stateMutex);

    // clear the stack
    lua_settop(L, 0);

    // load the method from the class definition
    lua_getglobal(L, "Parser");
	lua_getfield(L, -1, "doLogin");

	// pass the reference to the create object as the 1st param
	lua_rawgeti(L, LUA_REGISTRYINDEX, _stateObjIdx);
	
	// pass a login table as a param
	lua_newtable(L);
	lua_pushstring(L, info.first.toLatin1());
	lua_setfield(L, -2, "username");
	lua_pushstring(L, info.second.toLatin1());
	lua_setfield(L, -2, "password");

	// call Lua's function
	if (lua_pcall(L, 2, 1, 0) != 0)
	{
		QString strMsg = QString("problem calling 'doLogin' in %1: %2")
			.arg(_strLuaFile)
			.arg(lua_tostring(L, -1));
        _logger->error(strMsg.toStdString());

        OWL_THROW_EXCEPTION(OwlException(strMsg));
	}

	StringMap params = tableToParams(2);
	lua_pop(L, 1);
	lua_gc(L, LUA_GCCOLLECT, 0);

	return QVariant::fromValue(params);
}

QVariant LuaParserBase::doLogout()
{
    std::lock_guard<std::mutex> locker(*_stateMutex);

	// clear the stack
	lua_settop(L, 0);

	// load the method from the class definition
	lua_getglobal(L, "Parser");
	lua_getfield(L, -1, "doLogout");

	// pass the reference to the created object as the 1st param
	lua_rawgeti(L, LUA_REGISTRYINDEX, _stateObjIdx);

	if (lua_pcall(L, 1, 1, 0) != 0)
	{
        _logger->error("Lua call to doLogout failed: {}", lua_tostring(L, -1));
        OWL_THROW_EXCEPTION(LuaException(lua_tostring(L, -1)));
	}

	return QVariant::fromValue(tableToParams(2));
}

QVariant LuaParserBase::doGetBoardwareInfo()
{
    std::lock_guard<std::mutex> locker(*_stateMutex);
	owl::StringMap params;	

	// clear the stack
	lua_settop(L, 0);

	// load the method from the class definition
	lua_getglobal(L, "Parser");
	lua_getfield(L, -1, "doGetBoardwareInfo");

	// pass the reference to the create object as the 1st param
	lua_rawgeti(L, LUA_REGISTRYINDEX, _stateObjIdx);
	
	if (lua_pcall(L, 1, 1, 0) != 0)
	{
        _logger->error("Lua call to doGetBoardwareInfo failed: {}", lua_tostring(L, -1));
        OWL_THROW_EXCEPTION(LuaException(lua_tostring(L, -1)));
	}

	return QVariant::fromValue(tableToParams(2));
}

QVariant LuaParserBase::doGetForumList(const QString& forumId)
{
    std::lock_guard<std::mutex> locker(*_stateMutex);
	ForumList retval;

	// clear the stack
	lua_settop(L, 0);
    
	// load the method from the class definition
	lua_getglobal(L, "Parser");
	lua_getfield(L, -1, "doGetForumList");
    
	// pass the reference to the created object as the 1st param
	lua_rawgeti(L, LUA_REGISTRYINDEX, _stateObjIdx);
    
    // push the forumId
    lua_pushstring(L, forumId.toLatin1());
	
	if (lua_pcall(L, 2, 1, 0) != 0)
	{
        _logger->error("Lua call to doGetForumList failed: {}", lua_tostring(L, -1));
        OWL_THROW_EXCEPTION(LuaException(lua_tostring(L, -1)));
	}

	int tablePos = -1;
	int top = lua_gettop(L);
	if (tablePos < 0) tablePos += top + 1;

	if (!lua_isnil(L, tablePos) && lua_istable(L, tablePos))
	{
		lua_pushnil(L);
		while (lua_next(L, tablePos))
		{
			lua_gettop(L);
			StringMap info = tableToParams(lua_gettop(L));
			lua_pop(L, 1);

			if (info.has("forumId") && info.has("forumName") && info.has("forumType"))
			{
				ForumPtr newForum = ForumPtr(new Forum(info.getText("forumId")));
				newForum->setName(info.getText("forumName"));
				newForum->setForumType((Forum::ForumType)info.get<std::uint8_t>("forumType"));
                newForum->setHasUnread(info.getBool("forumUnread"));
				newForum->setDisplayOrder(info.get<std::uint8_t>("forumDisplayOrder"));

				if (newForum->getForumType() == Forum::LINK 
					&& info.has("forumLink"))
				{
					newForum->setVar("link", info.getText("forumLink"));
				}
                
				retval.push_back(newForum);
			}
		}
	}

	lua_pop(L, 1);

	return QVariant::fromValue(retval);
}

QVariant LuaParserBase::doThreadList(ForumPtr forumInfo, int options)
{
    std::lock_guard<std::mutex> locker(*_stateMutex);
    ThreadList retval;
    
	// clear the stack
	lua_settop(L, 0);
    
	// load the method from the class definition
	lua_getglobal(L, "Parser");
	lua_getfield(L, -1, "doThreadList");
    
	// pass the reference to the create object as the 1st param
	lua_rawgeti(L, LUA_REGISTRYINDEX, _stateObjIdx);
    
    // push the forumId
    lua_pushstring(L, forumInfo->getId().toLatin1());
    lua_pushnumber(L, forumInfo->getPageNumber());
    lua_pushnumber(L, forumInfo->getPerPage());
    lua_pushboolean(L, options & ParserEnums::REQUEST_NOCACHE);
	
	if (lua_pcall(L, 5, 1, 0) != 0)
	{
        _logger->error("Lua error: {}", lua_tostring(L, -1));
		 //throw LuaExceptionExt("Lua call to `doThreadList` failed", lua_tostring(L, -1));
        OWL_THROW_EXCEPTION(LuaException(lua_tostring(L,1)));
	}
    
	int tablePos = -1;
	int top = lua_gettop(L);
	if (tablePos < 0) tablePos += top + 1;
    
	if (!lua_isnil(L, tablePos) && lua_istable(L, tablePos))
	{
		lua_pushnil(L);
		while (lua_next(L, tablePos))
		{
			if (lua_isnumber(L, -2))
			{
				lua_gettop(L);
				StringMap info = tableToParams(lua_gettop(L));
				lua_pop(L, 1);

				if (info.has("threadId") && info.has("threadTitle") && info.has("threadAuthor"))
				{
					try
					{
						ThreadPtr newThread(new Thread(info.getText("threadId")));
						newThread->setTitle(info.getText("threadTitle"));
						newThread->setAuthor(info.getText("threadAuthor"));
						newThread->setHasUnread(info.getBool("threadHasUnread"));
						newThread->setSticky(info.getBool("threadIsSticky"));
						newThread->setPreviewText(info.getText("threadPreviewText"));

						PostPtr post(new Post(info.getText("threadLastPostId")));
						post->setAuthor(info.getText("threadLastPostAuthor"));
						post->setDatelineString(QString("%1 %2")
							.arg(info.getText("threadLastPostDate"))
							.arg(info.getText("threadLastPostTime")));

						bool bOk = false;
                        QDate date = _dtParser.parseDate(info.getText("threadLastPostDate").trimmed(), &bOk);
						if (bOk)
						{
							bOk = false;
                            QTime time = _dtParser.parseTime(info.getText("threadLastPostTime").trimmed(), &bOk);
							if (bOk)
							{
								post->setDateTime(QDateTime(date, time));
							}
						}

                        // get the reply count
                        if (info.has("threadReplyCount") && info.getText("threadReplyCount").size() > 0)
                        {
                            bOk = false;
                            QString temp = info.getText("threadReplyCount");
                            auto replycount = temp.replace(",", QString()).toUInt(&bOk);
                            if (bOk)
                            {
                                newThread->setReplyCount(replycount);
                            }
                        }

						newThread->setLastPost(post);
						newThread->setParent(forumInfo);

						retval.push_back(newThread);
					}
					catch (const StringMapException& ex)
					{
                        _logger->warn("Ignoring threadId `{}` - {}",
                            info.getText("threadId").toStdString(), ex.message().toStdString());
					}
				}
			}
			else if (lua_isstring(L, -2))
			{
				QString key(lua_tostring(L, -2));

				if (key.compare("#forumInfo") == 0)
				{
					lua_gettop(L);
					StringMap info = tableToParams(lua_gettop(L));
					lua_pop(L, 1);

					if (info.has("pageCount"))
					{
						forumInfo->setPageCount(info.get<std::uint8_t>("pageCount"));
					}
				}
			}
		}
	}
    
	lua_pop(L, 1);

    forumInfo->getThreads().clear();
    forumInfo->getThreads().append(retval);
    
    return QVariant::fromValue(forumInfo);
}

QVariant LuaParserBase::doGetPostList(ThreadPtr threadInfo, PostListOptions listOption, int webOptions)
{
    std::lock_guard<std::mutex> locker(*_stateMutex);
	PostList retval;

	// clear the stack
	lua_settop(L, 0);

	// load the method from the class definition
	lua_getglobal(L, "Parser");

	if (listOption == PostListOptions::FIRST_UNREAD)
	{
		lua_getfield(L, -1, "doUnreadPostList");

		// pass the reference to the create object as the 1st param
		lua_rawgeti(L, LUA_REGISTRYINDEX, _stateObjIdx);

		// pass the threadId and noReload options to the Lua function
		lua_pushstring(L, threadInfo->getId().toLatin1());
		lua_pushboolean(L, webOptions & ParserEnums::REQUEST_NOCACHE);

        _logger->trace("Lua call to 'doUnreadPostList'");
		if (lua_pcall(L, 3, 1, 0) != 0)
		{
            const std::string error("Lua call to 'doUnreadPostList' failed: " + std::string(lua_tostring(L, -1)));

            _logger->error(error);
            OWL_THROW_EXCEPTION(LuaException(QString::fromStdString(error)));
		}
	}
	else
	{
		lua_getfield(L, -1, "doPostList");
		// pass the reference to the create object as the 1st param
		lua_rawgeti(L, LUA_REGISTRYINDEX, _stateObjIdx);

		// push the forumId
		lua_pushstring(L, threadInfo->getId().toLatin1());
		lua_pushnumber(L, threadInfo->getPageNumber());
		lua_pushnumber(L, threadInfo->getPerPage());
		lua_pushboolean(L, webOptions & ParserEnums::REQUEST_NOCACHE);

        _logger->trace("Lua call to 'doPostList'");
		if (lua_pcall(L, 5, 1, 0) != 0)
		{
            _logger->error("Lua call to `doPostList` failed: {}", lua_tostring(L, -1));
            OWL_THROW_EXCEPTION(LuaException(lua_tostring(L, -1)));
		}
	}

	int tablePos = -1;
	int top = lua_gettop(L);
	if (tablePos < 0) tablePos += top + 1;

	if (!lua_isnil(L, tablePos) && lua_istable(L, tablePos))
	{
		QString firstUnreadId;

		lua_pushnil(L);
		while (lua_next(L, tablePos))
		{
			if (lua_isnumber(L, -2))
			{
				lua_gettop(L);
				StringMap info = tableToParams(lua_gettop(L));
				lua_pop(L, 1);

				if (info.has("post.id") && info.has("post.username"))
				{
					PostPtr post(new Post(info.getText("post.id")));
					post->setAuthor(info.getText("post.username"));
					post->setDatelineString(info.getText("post.timestamp"));

                    bool bOk = false;
                    auto date = _dtParser.parseDate(info.getText("post.date").trimmed(), &bOk);
                    if (bOk)
                    {
                        bOk = false;
                        auto time = _dtParser.parseTime(info.getText("post.time").trimmed(), &bOk);
                        if (bOk)
                        {
                            post->setDateTime(QDateTime(date, time));
                        }
                    }

					post->setText(info.getText("post.text"));
					post->setParent(threadInfo);
					retval.push_back(post);
				}
			}
			else if (lua_isstring(L, -2))
			{
				QString key(lua_tostring(L, -2));

				if (key.compare("#threadInfo") == 0)
				{
					lua_gettop(L);
					StringMap info = tableToParams(lua_gettop(L));
					lua_pop(L, 1);

					if (info.has("pageCount"))
					{
						threadInfo->setPageCount(info.get<std::uint8_t>("pageCount"));
					}

					if (info.has("perPage"))
					{
						threadInfo->setPerPage(info.get<std::uint8_t>("perPage"));
					}

					if (info.has("pageNum"))
					{
						threadInfo->setPageNumber(info.get<std::uint8_t>("pageNum"));
					}

					if (info.has("firstUnreadId"))
					{
						firstUnreadId = info.getText("firstUnreadId");
					}
				}
			}
		}

        // Go through and set the index for each post and the firstUnreadPost property
        // Page 1 = #1, Page 2 @ 25 pp = #26
        int index = ((threadInfo->getPageNumber()-1) * threadInfo->getPerPage()) + 1;
        for (auto p : retval)
        {
            p->setIndex(index++);
            if (!firstUnreadId.isEmpty() && p->getId() == firstUnreadId)
            {
                threadInfo->setFirstUnreadPost(p);
            }
        }
	}

	lua_pop(L, 1);

	threadInfo->getPosts().clear();
	threadInfo->getPosts().append(retval);

	return QVariant::fromValue(threadInfo);
}

QVariant LuaParserBase::doSubmitNewThread(ThreadPtr threadInfo)
{
    std::lock_guard<std::mutex> locker(*_stateMutex);
	ThreadPtr ret;

	// clear the stack
	lua_settop(L, 0);

	// load the method from the class definition
	lua_getglobal(L, "Parser");
	lua_getfield(L, -1, "doSubmitNewThread");

	// pass the reference to the create object as the 1st param
	lua_rawgeti(L, LUA_REGISTRYINDEX, _stateObjIdx);

	// pass a login table as a param
	lua_newtable(L);
	
	lua_pushstring(L, threadInfo->getParent()->getId().toLatin1());
	lua_setfield(L, -2, "forumId");

	lua_pushstring(L, threadInfo->getTitle().toLatin1());
	lua_setfield(L, -2, "title");
	
	lua_pushstring(L, threadInfo->getPosts().at(0)->getText().toLatin1());
	lua_setfield(L, -2, "text");

    lua_pushstring(L, ((QString)(threadInfo->getTags())).toLatin1());
	lua_setfield(L, -2, "taglist");

	// call Lua's function
	if (lua_pcall(L, 2, 1, 0) != 0)
	{
		QString strMsg = QString("problem calling 'doSubmitNewThread' in %1: %2")
			.arg(_strLuaFile)
			.arg(lua_tostring(L, -1));
        _logger->error(strMsg.toStdString());

        OWL_THROW_EXCEPTION(OwlException(strMsg));
	}

	if (lua_isstring(L, -1))
	{
		ret.reset();
		ret = ThreadPtr(new Thread(lua_tostring(L, -1)));
	}
    else
	{
        _logger->error("No threadId returned from doSubmitNewThread(), threadId = {}",
            threadInfo->getId().toStdString());
	}

	return QVariant::fromValue(ret);
}

QVariant LuaParserBase::doSubmitNewPost(PostPtr postInfo)
{
    std::lock_guard<std::mutex> locker(*_stateMutex);
		
	// clear the stack
	lua_settop(L, 0);

	// load the method from the class definition
	lua_getglobal(L, "Parser");
	lua_getfield(L, -1, "doSubmitNewPost");

	// pass the reference to the create object as the 1st param
	lua_rawgeti(L, LUA_REGISTRYINDEX, _stateObjIdx);

	// pass a login table as a param
	lua_newtable(L);
	lua_pushstring(L, postInfo->getParent()->getId().toLatin1());
	lua_setfield(L, -2, "parentId");
	lua_pushstring(L, postInfo->getTitle().toLatin1());
	lua_setfield(L, -2, "postTitle");
	lua_pushstring(L, postInfo->getText().toLatin1());
	lua_setfield(L, -2, "postText");

	// call Lua's function
	if (lua_pcall(L, 2, 1, 0) != 0)
	{
		QString strMsg = QString("problem calling 'doSubmitNewPost' in %1: %2")
			.arg(_strLuaFile)
			.arg(lua_tostring(L, -1));
        _logger->error(strMsg.toStdString());

        OWL_THROW_EXCEPTION(OwlException(strMsg));
	}

	PostPtr retPost;

	if (lua_isstring(L, -1))
	{
		retPost = PostPtr(new Post(lua_tostring(L, -1)));
		postInfo->getParent()->addChild(retPost);
	}
	else
	{
#ifdef _DEBUG
        lua::dumpStack(L);
#endif
        _logger->warn("LuaParser (%1) returned bad result from doSubmitNewPost()", getName().toStdString());
	}

	return QVariant::fromValue(retPost);
}

QVariant LuaParserBase::doMarkForumRead(ForumPtr forumInfo)
{
    std::lock_guard<std::mutex> locker(*_stateMutex);
	owl::StringMap params;	

	// clear the stack
	lua_settop(L, 0);

	// load the method from the class definition
	lua_getglobal(L, "Parser");
	lua_getfield(L, -1, "doMarkForumRead");

	// pass the reference to the create object as the 1st param
	lua_rawgeti(L, LUA_REGISTRYINDEX, _stateObjIdx);

	lua_pushstring(L, forumInfo->getId().toLatin1());
	
	if (lua_pcall(L, 2, 1, 0) != 0)
	{
        _logger->error("Lua call to doMarkForumRead failed: {}", lua_tostring(L, -1));
        OWL_THROW_EXCEPTION(LuaException(lua_tostring(L, -1)));
	}

	if (!lua_toboolean(L, -1))
	{
        _logger->error("Lua call to 'doMarkForumRead()' failed: {}", lua_tostring(L, -1));
        OWL_THROW_EXCEPTION(LuaException(lua_tostring(L, -1)));
	}

	return QVariant::fromValue(forumInfo);
}

QVariant LuaParserBase::doGetUnreadForums()
{
    std::lock_guard<std::mutex> locker(*_stateMutex);
	ForumList retval;

	// clear the stack
	lua_settop(L, 0);
    
	// load the method from the class definition
	lua_getglobal(L, "Parser");
	lua_getfield(L, -1, "doGetUnreadForums");
    
	// pass the reference to the create object as the 1st param
	lua_rawgeti(L, LUA_REGISTRYINDEX, _stateObjIdx);

	if (lua_pcall(L, 1, 1, 0) != 0)
	{
        _logger->error("Lua call to `doGetUnreadForums` failed: {}", lua_tostring(L, -1));
        OWL_THROW_EXCEPTION(LuaException(lua_tostring(L, -1)));
	}

	int tablePos = -1;
	int top = lua_gettop(L);
	if (tablePos < 0) tablePos += top + 1;

	if (!lua_isnil(L, tablePos) && lua_istable(L, tablePos))
	{
		lua_pushnil(L);
		while (lua_next(L, tablePos))
		{
			lua_gettop(L);
			StringMap info = tableToParams(lua_gettop(L));
			lua_pop(L, 1);

			if (info.has("forumId") && info.has("forumName") && info.has("forumType"))
			{
				ForumPtr newForum = ForumPtr(new Forum(info.getText("forumId")));
				newForum->setName(info.getText("forumName"));
				newForum->setForumType((Forum::ForumType)info.get<std::uint8_t>("forumType"));
                newForum->setHasUnread(info.getBool("forumUnread"));

				if (newForum->getForumType() == Forum::LINK 
					&& info.has("forumLink"))
				{
					newForum->setVar("link", info.getText("forumLink"));
				}
                
				retval.push_back(newForum);
			}
		}
	}

	lua_pop(L, 1);
	
	return QVariant::fromValue(retval);
}

StringMap LuaParserBase::tableToParams(int tablePos)
{
	StringMap params;

	lua_pushnil(L);
	while (lua_next(L, tablePos) != 0) 
	{
		int t = lua_type(L, -1);

		switch (t)
		{
			case LUA_TBOOLEAN:
				params.add(
					QString(lua_tostring(L, -2)),
					(bool)lua_toboolean(L, -1));
			break;

			case LUA_TNUMBER:
				params.add(
					QString(lua_tostring(L, -2)),
					(int)lua_tonumber(L, -1));
			break;

			case LUA_TSTRING:
				params.add(
					QString(lua_tostring(L, -2)),
					QString(lua_tostring(L, -1)));
			break;

			default:
                _logger->warn("unsupported type in lua table");
			break;
		}

		lua_pop(L, 1);
	}

	return params;
}

QVariant LuaParserBase::doGetEncryptionSettings()
{
    std::lock_guard<std::mutex> locker(*_stateMutex);
	owl::StringMap params;	

	// clear the stack
	lua_settop(L, 0);

	// load the method from the class definition
	lua_getglobal(L, "Parser");
	lua_getfield(L, -1, "doGetEncryptionSettings");

	// pass the reference to the create object as the 1st param
	lua_rawgeti(L, LUA_REGISTRYINDEX, _stateObjIdx);

	if (lua_pcall(L, 1, 1, 0) != 0)
	{
        _logger->error("Lua call to doGetEncryptionSettings failed: {}", lua_tostring(L, -1));
        OWL_THROW_EXCEPTION(LuaException(lua_tostring(L, -1)));
	}

	return QVariant::fromValue(tableToParams(2));
}

QVariant LuaParserBase::doTestParser(const QString& html)
{
    std::lock_guard<std::mutex> locker(*_stateMutex);
	owl::StringMap params;	

	// clear the stack
	lua_settop(L, 0);

	// load the method from the class definition
	lua_getglobal(L, "Parser");
	lua_getfield(L, -1, "doTestParser");

	// pass the reference to the create object as the 1st param
	lua_rawgeti(L, LUA_REGISTRYINDEX, _stateObjIdx);
	lua_pushstring(L, html.toLatin1());

	if (lua_pcall(L, 2, 1, 0) != 0)
	{
        _logger->error("Lua call to doTestParser failed: {}", lua_tostring(L, -1));
        OWL_THROW_EXCEPTION(LuaException(lua_tostring(L, -1)));
	}

	return QVariant::fromValue(tableToParams(2));;
}

} // namespace
