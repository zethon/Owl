#include "../Utils/QSgml.h"
#include "../Utils/WebClient.h"
#include "../Utils/StringMap.h"
#include "LuaParserBase.h"
#include "OwlLua.h"

namespace owl
{
	
StringMap OwlLua::tableToParams(lua_State* L, int tablePos)
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
                OWL_THROW_EXCEPTION(LuaException("Unsupported type in lua table"));
			break;
		}

		lua_pop(L, 1);
	}

    return params;
}

int OwlLua::doBreak(lua_State*)
{
#ifdef QT_DEBUG
	//__asm int 3;
#endif

	return 0;
}

int OwlLua::doCDebug(lua_State* L)
{
	QString trace1;

	if (lua_isstring(L, 1))
	{
		QString string(luaL_checkstring(L, 1));
		lua_Debug ar;
		lua_getstack(L, 1, &ar);
		lua_getinfo(L, "nslS", &ar);
		trace1 = QString("%1:%2 - %3")
			.arg(ar.source)
			.arg(ar.currentline)
			.arg(string);

        qDebug() << trace1;
	}

#ifdef QT_DEBUG
	//__asm int 3;
#endif

	return 0;
}

int OwlLua::getWebPage(lua_State* L)
{
    WebClient	client;
	QString		pageSrc;
	int			status = 200;
	bool		bIsError = false;
	QString strUrl(luaL_checkstring(L, 1));
	QString strMethod(lua_tostring(L, 2));
	QString strPostData(lua_tostring(L, 3));
	
	if (strMethod.isEmpty())
	{
		strMethod.append("GET");
	}

	try
	{
		if (strMethod == "GET")
		{
			pageSrc = client.DownloadString(strUrl);
		}
		else if (strMethod == "POST")
		{
			pageSrc = client.UploadString(strUrl, strPostData);
		}
		else
		{
			QString strError = QString("invalid #2 param 'method' in getWebPage: '%1'").arg(strMethod);
            OWL_THROW_EXCEPTION(LuaException(strError));
		}
	}
	catch (const WebException& ex)
	{
		bIsError = true;
		status = ex.statuscode();
	}

	lua_pushstring(L, pageSrc.toLatin1());
	lua_pushnumber(L, status);
	lua_pushboolean(L, bIsError);

	return 3;
}

int OwlLua::getMD5String(lua_State* L)
{
	QString string(luaL_checkstring(L, 1));
	QByteArray data(string.toLatin1());
	QString md5Pw = QString(QCryptographicHash::hash(data,QCryptographicHash::Md5).toHex());
	lua_pushstring(L, md5Pw.toLatin1());
	return 1;
}

int OwlLua::percentEncode(lua_State* L)
{
	QString string(luaL_checkstring(L, 1));
	QString encoded(QUrl::toPercentEncoding(string));
	lua_pushstring(L,encoded.toLatin1());
	return 1;
}

int OwlLua::stripHtml(lua_State* L)
{
	QString string(luaL_checkstring(L, 1));
	QTextDocument doc;
	doc.setHtml(string);
	lua_pushstring(L, doc.toPlainText().toLatin1());
	return 1;
}

//////////////////////////////////////////
// error Library    

int OwlLua::SgmlErrorWarn(lua_State* L)
{
	lua_Debug ar;
	lua_getstack(L, 1, &ar);
	lua_getinfo(L, "nslS", &ar);

    StringMap params = tableToParams(L, 1);
    OWL_THROW_EXCEPTION(LuaParserException(params, ar.source, ar.currentline));

	// should never get called
	return 0;
}

// Called from within Lua scripts as
//	error.throw(table)
int OwlLua::SgmlErrorThrow(lua_State* L)
{
	lua_Debug ar;
	lua_getstack(L, 1, &ar);
	lua_getinfo(L, "nslS", &ar);

    StringMap params = tableToParams(L, 1);
    OWL_THROW_EXCEPTION(LuaParserException(params, ar.source, ar.currentline));

	// should never get called
	return 0;
}

int OwlLua::newWebClient(lua_State* L)
{
	lua_getglobal(L, "__parserObj");
	Q_ASSERT(lua_islightuserdata(L, -1));

	ParserBase* parser = static_cast<ParserBase*>(lua_touserdata(L, -1));
	Q_ASSERT(parser != nullptr);    

    std::size_t size = sizeof(WebClient*);

	// create the new WebClient
    WebClient** data = static_cast<WebClient**>(lua_newuserdata(L, size));
    *data = new WebClient();
	(*data)->setConfig(parser->createWebClientConfig());

	// register the webclient object as a watcher of the parser settings
	parser->addWatcher(*data);

//    // set the cookie jar for our new webobject
//    (*data)->setCookieJar(parser->getCookieJar());

	// set the metatable
	luaL_setmetatable(L, "Owl.webclient");

	return 1;
}

int OwlLua::WebClientGet(lua_State* L)
{
    WebClient* client = checkWebClient(L);
	QString url;
	bool bSkipCache = false;
	
	if (lua_isboolean(L, -1))
	{
		bSkipCache = lua_toboolean(L, -1);
		url = luaL_checkstring(L, -2);
	}
	else
	{
		url = luaL_checkstring(L, -1);
	}

    WebClient::Options options = WebClient::DEFAULT;
	if (bSkipCache)
	{
        options = WebClient::NOCACHE;
	}

	QString		pageSrc;
	int			status = 200;
	bool		bIsError = false;

	try
	{
		pageSrc = client->DownloadString(url, options);
	}
	catch (const WebException& ex)
	{
		bIsError = true;
		status = ex.statuscode();
	}

	lua_pushstring(L, pageSrc.toLatin1());
	lua_pushnumber(L, status);
	lua_pushboolean(L, bIsError);
	lua_gc(L, LUA_GCCOLLECT, 0);
		
	return 3;
}

int OwlLua::WebClientGetRaw(lua_State* L)
{
    WebClient* client = checkWebClient(L);
	QString url;

	url = luaL_checkstring(L, -1);

	QString		pageSrc;
	int			status = 200;
	bool		bIsError = false;

	try
	{
        pageSrc = client->DownloadString(url, WebClient::NOTIDY |
                                         WebClient::NOENCRYPT |
                                         WebClient::NOCACHE);
	}
	catch (const WebException& ex)
	{
		bIsError = true;
		status = ex.statuscode();
	}

	lua_pushstring(L, pageSrc.toLatin1());
	lua_pushnumber(L, status);
	lua_pushboolean(L, bIsError);
	lua_gc(L, LUA_GCCOLLECT, 0);

	return 3;
}

// webclient:post(url,payload,skipCache)
// url			- (string) of the url to post to
// payload		- (string) the payload to put into the POST 
// [skipCache]	- (boolean) whether or not to skip the webClient cache
// Returns:
// html			- (string) HTML source of the requested page
// stats		- (int) HTTP status code
// isError		- (boolean) true if there was an error
int OwlLua::WebClientPost(lua_State* L)
{
    WebClient* client = checkWebClient(L);
	QString url,payload;
	bool bSkipCache = false;
	
	if (lua_isboolean(L, -1))
	{
		bSkipCache = lua_toboolean(L, -1);
		payload = luaL_checkstring(L, -2);
		url = luaL_checkstring(L, -3);
	}
	else
	{
		payload = luaL_checkstring(L, -1);
		url = luaL_checkstring(L, -2);
	}

    WebClient::Options options = WebClient::DEFAULT;
	if (bSkipCache)
	{
        options = WebClient::NOCACHE;
	}

	QString		pageSrc;
	int			status = 200;
	bool		bIsError = false;

	try
	{
		pageSrc = client->UploadString(url, payload, options);
	}
	catch (const WebException& ex)
	{
		bIsError = true;
		status = ex.statuscode();
	}

	lua_pushstring(L, pageSrc.toLatin1());
	lua_pushnumber(L, status);
	lua_pushboolean(L, bIsError);
	lua_gc(L, LUA_GCCOLLECT, 0);

	return 3;
}

int OwlLua::WebClientPostRaw(lua_State* L)
{
    WebClient* client = checkWebClient(L);

	QString payload = luaL_checkstring(L, -1);
	QString url = luaL_checkstring(L, -2);

	QString		pageSrc;
	int			status = 200;
	bool		bIsError = false;

	try
	{
		pageSrc = client->UploadString(url, payload, 
            WebClient::NOTIDY |
            WebClient::NOENCRYPT |
            WebClient::NOCACHE);
	}
	catch (const WebException& ex)
	{
		bIsError = true;
		status = ex.statuscode();
	}

	lua_pushstring(L, pageSrc.toLatin1());
	lua_pushnumber(L, status);
	lua_pushboolean(L, bIsError);
	lua_gc(L, LUA_GCCOLLECT, 0);

	return 3;
}

int OwlLua::WebClientGetLastUrl(lua_State* L)
{
    WebClient* client = checkWebClient(L);
	lua_pushstring(L, client->getLastRequestUrl().toLatin1());
	return 1;
}

int OwlLua::WebClientDestructor(lua_State* L)
{
    WebClient* client = checkWebClient(L);

	lua_getglobal(L, "__parserObj");
	Q_ASSERT(lua_islightuserdata(L, -1));

	ParserBase* parser = static_cast<ParserBase*>(lua_touserdata(L, -1));
	Q_ASSERT(parser != nullptr);

	parser->removeWatcher(client);
	
	delete client;

	return 1;
}

WebClient* OwlLua::checkWebClient(lua_State* L, int index/* = 1*/)
{
    auto temp = static_cast<WebClient**>(luaL_checkudata(L, index, "Owl.webclient"));
    return *temp;
}

/////////////////////////////////////////////////////////////////////
// QRegularExpression methods
/////////////////////////////////////////////////////////////////////

int OwlLua::newRegEx(lua_State* L)
{
	QString strPattern;
	bool bCheckCase = false;
    std::size_t size = sizeof(QRegularExpression*);

	if (lua_isboolean(L, -1))
	{
		bCheckCase = !lua_toboolean(L, -1);
		strPattern = luaL_checkstring(L, -2);
	}
	else
	{
		strPattern = luaL_checkstring(L, -1);		
	}

    QRegularExpression::PatternOption options = QRegularExpression::NoPatternOption;
    if (!bCheckCase) options = QRegularExpression::CaseInsensitiveOption;

    QRegularExpression** data = static_cast<QRegularExpression**>(lua_newuserdata(L, size));
	*data = new QRegularExpression(strPattern, options);

	luaL_setmetatable(L, "Owl.regexp");

	return 1;
}

int OwlLua::RegExIndexIn(lua_State* L)
{
	// QRegularExpression* exp = checkRegExp(L);

	// if (lua_isnumber(L, -1))
	// {
	// 	QString searchStr(luaL_checkstring(L, -2));
	// 	lua_pushnumber(L, exp->indexIn(searchStr, luaL_checkint(L, -1)));
	// }
	// else
	// {
	// 	QString searchStr(luaL_checkstring(L, -1));
	// 	lua_pushnumber(L, exp->indexIn(searchStr));
	// }

	return 1;
}

int OwlLua::RegExCap(lua_State* L)
{
	// QRegularExpression* exp = checkRegExp(L);

	// if (lua_isnumber(L, -1))
	// {
    //     int iIdx = lua_tonumber(L, -1);
		
	// 	if (iIdx > 0 && iIdx >= exp->captureCount())
	// 	{
	// 		lua_pushstring(L, exp->cap(iIdx).toLatin1());
	// 	}
	// }

	return 1;
}
    
int OwlLua::RegExMatchedLength(lua_State* L)
{
	// QRegularExpression* exp = checkRegExp(L);
    // lua_pushnumber(L, exp->matchedLength());

    return 1;    
}

int OwlLua::RegExDestructor(lua_State* L)
{
	QRegularExpression* regex = checkRegExp(L);
	delete regex;

	return 0;
}

QRegularExpression* OwlLua::checkRegExp(lua_State* L, int index)
{
    auto temp = static_cast<QRegularExpression**>(luaL_checkudata(L, index, "Owl.regexp"));
    return *temp;
}

/////////////////////////////////////////////////////////////////////
// QSgml methods
/////////////////////////////////////////////////////////////////////

int OwlLua::newSgml(lua_State* L)
{
	int size = sizeof(QSgml*);

	if (lua_isstring(L, -1))
	{
		// the text to parse can be passed to the constructor
		QString docSrc(luaL_checkstring(L, -1));

		// create the new QSgml object
		QSgml** data = (QSgml**)lua_newuserdata(L, size);
		*data = new QSgml(docSrc);
	}
	else
	{
		QSgml** data = (QSgml**)lua_newuserdata(L, size);
		*data = new QSgml();
	}

	luaL_setmetatable(L, "Owl.sgml");

	return 1;
}

int OwlLua::SgmlParse(lua_State* L)
{
	QSgml* doc = checkSgml(L);
	QString docSrc(luaL_checkstring(L, -1));

	lua_pushboolean(L, doc->parse(docSrc));
	return 1;
}

int OwlLua::SgmlGetElementsByName(lua_State* L)
{
	QList<QSgmlTag*> tags;
	QString element;
	QString attribute;
	QSgml* exp = checkSgml(L);

	if (lua_isstring(L, -3))
	{
		element = QString::fromLatin1(lua_tostring(L, -3));
	}

	if (lua_isstring(L, -2))
	{
		attribute = QString::fromLatin1(lua_tostring(L, -2));
	}

	// the first arg is required
	if (!element.isEmpty())
	{
		if (lua_isstring(L, -1))
		{
			QString value(luaL_checkstring(L, -1));

			// doc:getElementsByName("meta", "name", "copyright")
			exp->getElementsByName(element, attribute, value, &tags);
		}
		else if (lua_isuserdata(L, -1))
		{
			// local reg = regexp.new("sid=([a-zA-Z0-9]+)")
			// local tags = doc:getElementsByName("meta", "name", reg)
			QRegularExpression* reg = checkRegExp(L,-1);

            if (reg != nullptr)
			{
				exp->getElementsByName(element, attribute,(const QRegularExpression&)*reg, &tags);
			}
		}
		else 
		{
			// third argument is nil
			if (!attribute.isEmpty())
			{
				// doc:getElementsByName("foo","bar",nil)
				attribute = QString::fromLatin1(lua_tostring(L, -2));
				exp->getElementsByName(element, attribute, &tags);
			}
			else
			{
				// doc:getElementsByName("foo",nil,nil)
				exp->getElementsByName(element, &tags);
			}
		}
	}

	lua_newtable(L);
	int i = 1;
	int size = sizeof(QSgmlTag**);

    for (QSgmlTag* tag : tags)
	{
		lua_pushnumber(L, i);
		*((QSgmlTag**)lua_newuserdata(L, size)) = tag;
		luaL_setmetatable(L, "Owl.sgmltag");
		lua_settable(L, -3);
		i++;
	}

	return 1;
}

int OwlLua::SgmlDocTag(lua_State* L)
{
	QSgml* doc = checkSgml(L);
	QSgmlTag* docTag = doc->DocTag;

    if (docTag != nullptr)
	{
		int size = sizeof(QSgmlTag**);
		*((QSgmlTag**)lua_newuserdata(L, size)) = docTag;
		luaL_setmetatable(L, "Owl.sgmltag");
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

int OwlLua::SgmlDocGetText(lua_State* L)
{
	//QSgml* doc = (QSgml*)luaL_checkudata(L, 1, "Owl.sgml");
	QSgml* doc = checkSgml(L);
	QSgmlTag* tag = *((QSgmlTag**)luaL_checkudata(L, 2, "Owl.sgmltag"));
	QString strText;

    if (doc != nullptr && tag != nullptr)
	{
		doc->getText(tag, &strText);
		lua_pushstring(L, strText.toLatin1());
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

int OwlLua::SgmlGetDocText(lua_State* L)
{
	QSgml* doc = checkSgml(L);

	QString strHtml; 
	doc->ExportString(&strHtml);
	lua_pushstring(L, strHtml.toLatin1());

	return 1;
}

int OwlLua::SgmlDestructor(lua_State* L)
{
	QSgml* sgml = checkSgml(L);
	delete sgml;

	return 0;
}

QSgml* OwlLua::checkSgml(lua_State* L, int index)
{
	return *(QSgml**)luaL_checkudata(L, index, "Owl.sgml");
}

/////////////////////////////////////////////////////////////////////
// QSgmlTag methods
/////////////////////////////////////////////////////////////////////

int OwlLua::SgmlTagName(lua_State* L)
{
	QSgmlTag* tag = *((QSgmlTag**)luaL_checkudata(L, 1, "Owl.sgmltag"));
	lua_pushstring(L, tag->Name.toLatin1());
	return 1;
}

int OwlLua::SgmlTagAttribute(lua_State* L)
{
	QSgmlTag* tag = *((QSgmlTag**)luaL_checkudata(L, 1, "Owl.sgmltag"));

	QString attrName(luaL_checkstring(L, -1));
	QString attrVal;

	if (tag->hasAttribute(attrName))
	{
		attrVal = tag->Attributes.value(attrName);
	}

	lua_pushstring(L, attrVal.toLatin1());
	return 1;
}

int OwlLua::SgmlTagHasAttribute(lua_State* L)
{
	QSgmlTag* tag = *((QSgmlTag**)luaL_checkudata(L, 1, "Owl.sgmltag"));
	QString attrName(luaL_checkstring(L, -1));
	lua_pushboolean(L, tag->hasAttribute(attrName));
	return 1;
}

int OwlLua::SgmlTagSetAttribute(lua_State* L)
{
	QSgmlTag* tag = *((QSgmlTag**)luaL_checkudata(L, 1, "Owl.sgmltag"));

	QString attrName(luaL_checkstring(L, -2));
	QString attrVal(luaL_checkstring(L, -1));

	tag->Attributes[attrName] = attrVal;

	return 1;
}

int OwlLua::SgmlTagStartTagPos(lua_State* L)
{
	QSgmlTag* tag = *((QSgmlTag**)luaL_checkudata(L, 1, "Owl.sgmltag"));
	lua_pushnumber(L, tag->StartTagPos);
	return 1;
}

int OwlLua::SgmlTagStartTagLength(lua_State* L)
{
	QSgmlTag* tag = *((QSgmlTag**)luaL_checkudata(L, 1, "Owl.sgmltag"));
	lua_pushnumber(L, tag->StartTagLength);
	return 1;
}

int OwlLua::SgmlTagEndTagPos(lua_State* L)
{
	QSgmlTag* tag = *((QSgmlTag**)luaL_checkudata(L, 1, "Owl.sgmltag"));
	lua_pushnumber(L, tag->EndTagPos);
	return 1;
}

int OwlLua::SgmlTagEndTagLength(lua_State* L)
{
	QSgmlTag* tag = *((QSgmlTag**)luaL_checkudata(L, 1, "Owl.sgmltag"));
	lua_pushnumber(L, tag->EndTagLength);
	return 1;
}	

int OwlLua::SgmlTagChildren(lua_State* L)
{
    QSgmlTag* sgmltag = *((QSgmlTag**)luaL_checkudata(L, 1, "Owl.sgmltag"));
	lua_newtable(L);
	int i = 1;
	int size = sizeof(QSgmlTag**);

    for (QSgmlTag* tag : sgmltag->Children)
	{
		lua_pushnumber(L, i);
		*((QSgmlTag**)lua_newuserdata(L, size)) = tag;
		luaL_setmetatable(L, "Owl.sgmltag");
		lua_settable(L, -3);
		i++;
	}

	return 1;
}

int OwlLua::SgmlTagPrevious(lua_State* L)
{
	QSgmlTag* tag = *((QSgmlTag**)luaL_checkudata(L, 1, "Owl.sgmltag"));
	QSgmlTag* prev = tag->getPrevious();

    if (prev != nullptr)
	{
		int size = sizeof(QSgmlTag**);
		*((QSgmlTag**)lua_newuserdata(L, size)) = prev;
		luaL_setmetatable(L, "Owl.sgmltag");
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

int OwlLua::SgmlTagParent(lua_State* L)
{
	QSgmlTag* tag = *((QSgmlTag**)luaL_checkudata(L, 1, "Owl.sgmltag"));
	QSgmlTag* parent = tag->Parent;

    if (parent != nullptr)
	{
		int size = sizeof(QSgmlTag**);
		*((QSgmlTag**)lua_newuserdata(L, size)) = parent;
		luaL_setmetatable(L, "Owl.sgmltag");
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

int OwlLua::SgmlTagCompare(lua_State* L)
{
	QSgmlTag* tag1 = *((QSgmlTag**)luaL_checkudata(L, 1, "Owl.sgmltag"));
	QSgmlTag* tag2 = *((QSgmlTag**)luaL_checkudata(L, 2, "Owl.sgmltag"));
	lua_pushboolean(L, tag1 == tag2);
	return 1;
}

int OwlLua::SgmlTagValue(lua_State* L)
{
	QSgmlTag* tag = *((QSgmlTag**)luaL_checkudata(L, 1, "Owl.sgmltag"));

    if (tag != nullptr)
	{
		lua_pushstring(L, tag->Value.toLatin1());
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

namespace lua
{

void pushstring(lua_State * L, const std::string & s)
{
    lua_pushlstring(L, s.empty() ? "" : s.c_str(), s.size());
}

//const static luaL_Reg emptyLib[] = { { nullptr, nullptr } };

/**
 * [-0, +1, m]
 */
//void registerclosures(lua_State * L,
//					  const std::string & libname,
//					  const luaL_Reg * l,
//					  void * upvalue)
//{
//	luaL_register(L, libname.c_str(), emptyLib);					// +1
//
//	for (; l->name; ++l)
//	{
//		// lib[name] = cclosure(func, upvalue)
//		lua_pushstring(L, l->name);									// +1
//		lua_pushlightuserdata(L, upvalue);							// +1
//		lua_pushcclosure(L, l->func, 1);							// -1/+1
//		lua_rawset(L, -3);											// -2
//	}
//}

int absoluteindex(lua_State * L, int index)
{
    int top = lua_gettop(L);
    if (index < 0 && -index <= top)
    {
        index += top + 1;
    }
    return index;
}

/**
 * [-0, +0, e]
 */
bool checktable(lua_State *L, int narg, const char * tname)
{
    narg = lua::absoluteindex(L, narg);
    if (!lua_istable(L, narg)) return false;
    const int top = lua_gettop(L);
    bool result = false;
    if (!lua_getmetatable(L, narg))
    {
        if (tname == nullptr)
        {
            // no metatable and none specified
            result = true;
        }
    }
    else
    {
        luaL_getmetatable(L, tname);
        result = lua_rawequal(L, -1, -2) ? true : false;
    }
    lua_settop(L, top);
    return result;
}

void dumpStack(lua_State * L)
{
    // Grab the current top of the stack
    int top = lua_gettop(L);

    printf("total in stack %d\n",top);

    for (int i = 1; i <= top; i++)
    {
        printf("%d:", i);
        // Output the current stack item
        dumpItem(L, i, 1);

        // Add a space to putput
        printf("\n");  /* put a separator */
    }

    printf("\n");  /* end the listing */
}

void dumpTable(lua_State * L, int tablePos, int depth)
{
    int top = lua_gettop(L);

    if (tablePos < 0) tablePos += top + 1;

    if (!lua_isnil(L, tablePos) && lua_istable(L, tablePos))
    {
        if (lua_getmetatable(L, tablePos))
        {
            printf("metatable ");
            if (lua_rawequal(L, tablePos, -1))
            {
                printf("self\n");
                for (int i = 0; i < depth; ++i) printf("\t");
            }
            else
            {
                dumpTable(L, -1, depth + 1);
            }
        }

        printf("{");
        ++depth;

        // Table is in the stack at index 'index'
        lua_pushnil(L);  // first key

        bool empty = true;
        while (lua_next(L, tablePos) != 0)
        {
            if (!empty) printf(",");
            empty = false;
            printf("\n");

            // print key
            if (lua_istable(L, -2))
            {
                dumpTable(L, -2, depth);
            }
            else
            {
                dumpItem(L, -2, depth);
            }
            printf(" = ");
            // print value
            if (lua_istable(L, -1))
            {
                if (lua_rawequal(L, tablePos, -1))
                {
                    printf("self");
                }
                else
                {
                    printf("\n");
                    for (int i = 0; i < depth; ++i) printf("\t");
                    dumpTable(L, -1, depth);
                }
            }
            else
            {
                dumpItem(L, -1, 0);
            }

            // pop the value, leave the key
            lua_pop(L, 1);
        }

        // end the listing
        if (!empty)
        {
            printf("\n");
            for (int i = 0; i < depth - 1; ++i) printf("\t");
        }
        printf("}");
    }

    // Reset stack
    lua_settop(L, top);
}

void dumpItem(lua_State * L, int stackPos, int depth)
{
    // repeat for each level
    int t = lua_type(L, stackPos);

    if (stackPos < 0) stackPos += lua_gettop(L) + 1;

    for (int i = 0; i < depth; ++i) printf("\t");

    switch (t)
    {
        case LUA_TNIL:
            printf("nil");
            break;
        case LUA_TLIGHTUSERDATA:
            printf("lightuserdata");
            break;
        case LUA_TSTRING:
        {
            size_t length;
            const char * value = lua_tolstring(L, stackPos, &length);
            dumpString(value, length);
            break;
        }
        case LUA_TBOOLEAN:
            printf("%s",lua_toboolean(L, stackPos) ? "true" : "false");
            break;
        case LUA_TNUMBER:
            printf("%g", lua_tonumber(L, stackPos));
            break;
        case LUA_TTABLE:
            dumpTable(L, stackPos, depth);
            break;
        case LUA_TFUNCTION:
            printf("function");
            break;
        case LUA_TTHREAD:
            printf("thread");
            break;
        default:  /* other values */
            printf("Unknown type: %d", t);
            break;
    }
}

void dumpString(const char * str, size_t len)
{
    printf("'");
    for (size_t i = 0; i < len; ++i)
    {
        if (str[i] < 32)
        {
            printf("\\%u", static_cast<std::uint32_t>(str[i]));
        }
        else
        {
            printf("%c", str[i]);
        }
    }
    printf("'");
}

} // lua

} // namespace
