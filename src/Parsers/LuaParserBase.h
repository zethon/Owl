// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <mutex>
#include <setjmp.h>
#include <QtCore>
#include <lua.hpp>
#include <logger.h>
#include "../Utils/DateTimeParser.h"
#include "../Utils/StringMap.h"
#include "ParserBase.h"

namespace owl
{

#define LUA_PARSER_EXCEPTION_X(x,y,z)		LuaParserException(x, y, z, _SRCFILENAME, __LINE__)
#define LUA_PARSER_EXCEPTION_Y(l,s)			LuaParserException(l, s, _SRCFILENAME, __LINE__)

class LuaParserBase;
class LuaParserException;

using LuaParserBasePtr = std::shared_ptr<LuaParserBase>;
using LuaParserExceptionPtr = std::shared_ptr<LuaParserException>;
using MutexPtr = std::shared_ptr<std::mutex>;

// TODO: see if we need to pass the lua_State
class LuaParserException : public OwlException
{
public:
	LuaParserException() { }
	virtual ~LuaParserException() throw() { }

	LuaParserException& operator=(const LuaParserException other)
	{
		OwlException::operator=(other);

		_params.clear();
		_params.add(const_cast<owl::StringMap*>(other.getParams()));

		return *this;
	}

	LuaParserException(const QString& msg, const QString scriptFile = "", int line = 0)
		: OwlException(msg, scriptFile, line)
	{
		// do nothing
	}

	LuaParserException(
		const QString& msg,				
		const QString& data,			
		const QString& url,				
		const QString scriptFile = "",	
		int line = 0)					
		: OwlException(msg, scriptFile, line)
	{
        _params.add("data", data);
        _params.add("url", url);
	}

	LuaParserException(StringMap params, const QString& filename, const int iLine);

	
	virtual void raise() const { throw *this; }
	virtual LuaParserException* clone() const { return new LuaParserException(*this); }

	const StringMap* getParams() const { return &_params; }

private: 
	StringMap _params;
};
   
class LuaParserBase : public ParserBase
{
	Q_OBJECT
	LOG4QT_DECLARE_QCLASS_LOGGER

public:
	Q_INVOKABLE LuaParserBase(const QString& url, const QString& luaFile);
	virtual ~LuaParserBase();

	virtual QString getName() const;
	virtual QString getPrettyName() const;

	virtual QString getItemUrl(ForumPtr forum);
	virtual QString getItemUrl(ThreadPtr thread);
	virtual QString getItemUrl(PostPtr post);

	virtual QString getPostQuote(PostPtr post);

    virtual ParserBasePtr clone(ParserBasePtr other = ParserBasePtr()) override;

    virtual QString getLastRequestUrl() override;

protected:
	/* ParserBase implementation */
	virtual QVariant doLogin(const LoginInfo&);
	virtual QVariant doLogout();
	
	virtual QVariant doGetBoardwareInfo();
	virtual QVariant doTestParser(const QString&);

	virtual QVariant doGetForumList(const QString& forumId);
	virtual QVariant doThreadList(ForumPtr forumInfo, int options);
	
	//virtual QVariant doPostList(ThreadPtr threadInfo, int options);
	virtual QVariant doGetPostList(ThreadPtr t, PostListOptions listOption, int webOptions) override;

	virtual QVariant doSubmitNewThread(ThreadPtr threadInfo);
	virtual QVariant doSubmitNewPost(PostPtr postInfo);

	virtual QVariant doGetUnreadForums();
	virtual QVariant doMarkForumRead(ForumPtr forumInfo);

	virtual QVariant doGetEncryptionSettings();

private:
	void registerFunctions();
	StringMap tableToParams(int tablePos);
	QString getItemUrlHelper(const QString funcName, const QString itemId);

	DateTimeParser	_dtParser;

    QString         _strLuaFile;
    int             _stateObjIdx;
    bool            _doCloseState = true;

    lua_State*      L;
    MutexPtr        _stateMutex;
};

} // namespace
