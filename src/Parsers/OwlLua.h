// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <lua.hpp>

class QSgml;

namespace owl
{

class StringMap;
class WebClient;

class OwlLua 
{
	    
public:
	// helper methods
    static StringMap tableToParams(lua_State* L, int tablePos);

	// utils object
	static int doBreak(lua_State* L);
	static int doCDebug(lua_State* L);
	static int getWebPage(lua_State* L);
	static int getMD5String(lua_State* L);
	static int percentEncode(lua_State* L);
	static int stripHtml(lua_State* L);

	// webclient object
	static int newWebClient(lua_State* L);
	static int WebClientGet(lua_State* L);
	static int WebClientGetRaw(lua_State* L);
	static int WebClientPost(lua_State* L);
	static int WebClientPostRaw(lua_State* L);
	static int WebClientGetLastUrl(lua_State* L);
	static int WebClientDestructor(lua_State* L);
	
	// regex object
	static int newRegEx(lua_State* L);
	static int RegExIndexIn(lua_State* L);
	static int RegExCap(lua_State* L);
    static int RegExMatchedLength(lua_State* L);
	static int RegExDestructor(lua_State* L);

	// sgmldoc object
	static int newSgml(lua_State* L);
	static int SgmlParse(lua_State* L);
	static int SgmlGetElementsByName(lua_State* L);
	static int SgmlDocTag(lua_State* L);
	static int SgmlDocGetText(lua_State* L);
	static int SgmlGetDocText(lua_State* L);
	static int SgmlDestructor(lua_State* L);

	// sgmltag object
	static int SgmlTagName(lua_State* L);
	static int SgmlTagAttribute(lua_State* L);
	static int SgmlTagHasAttribute(lua_State* L);
	static int SgmlTagSetAttribute(lua_State* L);
	static int SgmlTagStartTagPos(lua_State* L);
	static int SgmlTagStartTagLength(lua_State* L);
	static int SgmlTagEndTagPos(lua_State* L);
	static int SgmlTagEndTagLength(lua_State* L);
	static int SgmlTagChildren(lua_State* L);	
	static int SgmlTagPrevious(lua_State* L);
	static int SgmlTagParent(lua_State* L);
	static int SgmlTagCompare(lua_State* L);
	static int SgmlTagValue(lua_State* L);
	
    static int SgmlErrorWarn(lua_State* L);
	static int SgmlErrorThrow(lua_State* L);	

private:
	static QSgml* checkSgml(lua_State* L, int index = 1);
	static QRegExp* checkRegExp(lua_State* L, int index = 1);
    static WebClient* checkWebClient(lua_State* L, int index = 1);
};

static const struct luaL_Reg webclientlib[] =
{
	{"new", OwlLua::newWebClient},
	{"get", OwlLua::WebClientGet},	
	{"getRaw", OwlLua::WebClientGetRaw},
	{"post", OwlLua::WebClientPost},
	{"postRaw", OwlLua::WebClientPostRaw},	
	{"getLastUrl", OwlLua::WebClientGetLastUrl},
	{"__gc", OwlLua::WebClientDestructor},
	{NULL, NULL}
};

static const struct luaL_Reg regexplib[] =
{
	{"new", OwlLua::newRegEx},
	{"indexIn", OwlLua::RegExIndexIn},
	{"cap", OwlLua::RegExCap},
    {"matchedlength",OwlLua::RegExMatchedLength},
	{"__gc", OwlLua::RegExDestructor},
	{NULL, NULL}
};

static const struct luaL_Reg sgmltaglib[] =
{
	{"name", OwlLua::SgmlTagName},
	{"attribute", OwlLua::SgmlTagAttribute},
	{"hasAttribute", OwlLua::SgmlTagHasAttribute},
	{"setAttribute", OwlLua::SgmlTagSetAttribute},
	{"startPos", OwlLua::SgmlTagStartTagPos},
	{"startLen", OwlLua::SgmlTagStartTagLength},
	{"endPos", OwlLua::SgmlTagEndTagPos},
	{"endLen", OwlLua::SgmlTagEndTagLength},
	{"children", OwlLua::SgmlTagChildren},
	{"previous", OwlLua::SgmlTagPrevious},
	{"parent", OwlLua::SgmlTagParent},
	{"compare", OwlLua::SgmlTagCompare},
	{"value", OwlLua::SgmlTagValue},
	{NULL, NULL}
};

static const struct luaL_Reg sgmllib[] =
{
	{"new", OwlLua::newSgml},
	{"parse", OwlLua::SgmlParse},
	{"getElementsByName", OwlLua::SgmlGetElementsByName},
	{"doctag", OwlLua::SgmlDocTag},
	{"getText", OwlLua::SgmlDocGetText},
	{"docText", OwlLua::SgmlGetDocText },
	{"__gc", OwlLua::SgmlDestructor},
	{NULL, NULL}
};

static const struct luaL_Reg owlutilslib[] = 
{
	{"break", OwlLua::doBreak},
	{"debug", OwlLua::doCDebug},
	{"md5", OwlLua::getMD5String},
	{"stripHtml", OwlLua::stripHtml},
	{"percentEncode", OwlLua::percentEncode},
	{NULL, NULL}
};
    
static const struct luaL_Reg errorlib[] =
{
    {"warn", OwlLua::SgmlErrorWarn},
    {"notify", NULL},
    {"throw", OwlLua::SgmlErrorThrow},
    {"panic", NULL},
    {NULL,NULL}
};

namespace lua
{
    template <typename T>
    T * checkudata(lua_State * L, int narg, const char * tname)
    {
        return static_cast<T *>(luaL_checkudata(L, narg, tname));
    }

    template <typename T>
    T * checkudata(lua_State * L, int narg)
    {
        return static_cast<T *>(luaL_checkudata(L, narg, T::className));
    }

    template <typename T>
    T * checkpudata(lua_State * L, int narg)
    {
        return *static_cast<T **>(luaL_checkudata(L, narg, T::className));
    }

    bool checktable(lua_State *L, int narg, const char * tname);

    //template <typename T, int(T::*MemberFunc)(lua_State *)>
    //int bind_closure(lua_State * L)
    //{
    //	const int uvi = lua_upvalueindex(1);
    //	T * t = reinterpret_cast<T *>(lua_touserdata(L, uvi));
    //	return (t->*MemberFunc)(L);
    //}

    void pushstring(lua_State * L, const std::string & s);

    //void registerclosures(lua_State * L,
    //						const std::string & libname,
    //						const luaL_Reg * l,
    //						void * upvalue);

    int absoluteindex(lua_State * L, int index);

    void dumpStack(lua_State * L);
    void dumpTable(lua_State * L, int tablePos, int depth = 0);
    void dumpItem(lua_State * L, int stackPos, int depth = 0);
    void dumpString(const char * value, size_t length);
} // lua

} // namespace
