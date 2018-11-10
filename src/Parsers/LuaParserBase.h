#pragma once
#include <mutex>
#include <setjmp.h>
#include <QtCore>
#include <lua/lua.hpp>
#include "../Utils/DateTimeParser.h"
#include "../Utils/StringMap.h"
#include "ParserBase.h"

namespace spdlog
{
    class logger;
}

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
		_params.merge(*(other.getParams()));

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

public:
	Q_INVOKABLE LuaParserBase(const QString& url, const QString& luaFile);
	virtual ~LuaParserBase();

        virtual QString getName() const override;
        virtual QString getPrettyName() const override;

        virtual QString getItemUrl(ForumPtr forum) override;
        virtual QString getItemUrl(ThreadPtr thread) override;
        virtual QString getItemUrl(PostPtr post) override;

        virtual QString getPostQuote(PostPtr post) override;

    virtual ParserBasePtr clone(ParserBasePtr other = ParserBasePtr()) override;

    virtual QString getLastRequestUrl() override;

protected:
	/* ParserBase implementation */
        virtual QVariant doLogin(const LoginInfo&) override;
        virtual QVariant doLogout() override;
	
        virtual QVariant doGetBoardwareInfo() override;
        virtual QVariant doTestParser(const QString&) override;

        virtual QVariant doGetForumList(const QString& forumId) override;
        virtual QVariant doThreadList(ForumPtr forumInfo, int options) override;
	
	//virtual QVariant doPostList(ThreadPtr threadInfo, int options);
	virtual QVariant doGetPostList(ThreadPtr t, PostListOptions listOption, int webOptions) override;

        virtual QVariant doSubmitNewThread(ThreadPtr threadInfo) override;
        virtual QVariant doSubmitNewPost(PostPtr postInfo) override;

        virtual QVariant doGetUnreadForums() override;
        virtual QVariant doMarkForumRead(ForumPtr forumInfo) override;

        virtual QVariant doGetEncryptionSettings() override;

private:
	void registerFunctions();
	StringMap tableToParams(int tablePos);
    QString getItemUrlHelper(const QString &funcName, const QString itemId);

	DateTimeParser	_dtParser;

    QString         _strLuaFile;
    int             _stateObjIdx;
    bool            _doCloseState = true;

    lua_State*      L;
    MutexPtr        _stateMutex;

    std::shared_ptr<spdlog::logger>  _logger;
};

} // namespace
