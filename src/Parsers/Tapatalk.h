// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <QtCore>
#include <logger.h>
#include "../Utils/StringMap.h"
#include "../Utils/xrvariant.h"
#include "ParserBase.h"

namespace owl
{

#define TAPATALK_NAME           "tapatalk4x"
#define TAPATALK_PRETTYNAME     "Tapatalk"

struct TapaTalkParam;

class Forum;
using ForumPtr = std::shared_ptr<Forum>;

using ForumMap = QMap<QString, ForumPtr>;

class Tapatalk4x;
using Tapatalk4xPtr = std::shared_ptr<Tapatalk4x>;

class Tapatalk4x : public ParserBase
{
	Q_OBJECT
	LOG4QT_DECLARE_QCLASS_LOGGER

public:
    static const uint LOGINTIMEOUT;

	enum ParamType
	{
		STRING,
		BYTE,
		BASE64,
		INT,
		BOOLEAN,
		STRUCT
	};

	struct TapaTalkParam 
	{
		ParamType type;
		QVariant value;
		QString name;

		TapaTalkParam() 
		{
		}

		TapaTalkParam(ParamType pType, const QString& pValue, QString pName = QString())
			: type(pType),
			  value(pValue),
			  name(pName)
		{
		}

		TapaTalkParam(ParamType pType, const QVariant& pValue, QString pName = QString())
			: type(pType),
			value(pValue),
			name(pName)
		{
		}
	};

	typedef QMap<QString, TapaTalkParam> ParamMap;
	typedef QList<TapaTalkParam> ParamList;

	Q_INVOKABLE Tapatalk4x(const QString& url);
	virtual ~Tapatalk4x();

	virtual const QString getRootForumId() const { return _rootId; }

    virtual ParserBasePtr clone(ParserBasePtr other = ParserBasePtr()) override
    {
        Tapatalk4xPtr retval = std::make_shared<Tapatalk4x>(getBaseUrl());
        ParserBase::clone(retval);

        retval->_rootId = _rootId;
        retval->_rootIdRealized = _rootIdRealized;
        retval->_configLoaded = _configLoaded;

        retval->_version = _version;
        retval->_bUseMD5 = _bUseMD5;
        retval->_bUseSha1 = _bUseSha1;
        retval->_apiLevel = _apiLevel;

        retval->_forumName = _forumName;
        retval->_forumMap = _forumMap;
        retval->_forumMapInitialized = _forumMapInitialized;

        retval->_loginInfo = _loginInfo;
        retval->_lastLogin = _lastLogin;

        retval->_webclient.setCurlHandle(_webclient.getCurlHandle());

        return retval;
    }

    virtual QString getPostQuote(PostPtr post) override;

    virtual QString getLastRequestUrl() override
    {
        return _webclient.getLastRequestUrl();
    }

protected:
	virtual QVariant doLogin(const LoginInfo&);
	virtual QVariant doLogout();
	
	virtual QVariant doGetBoardwareInfo();
	virtual QVariant doTestParser(const QString&);

	virtual QVariant doThreadList(ForumPtr forumInfo, int options);

	virtual QVariant doSubmitNewThread(ThreadPtr threadInfo);
	virtual QVariant doSubmitNewPost(PostPtr postInfo);
	virtual QVariant doGetForumList(const QString& forumId);
	virtual QVariant doGetEncryptionSettings();
	virtual QVariant doMarkForumRead(ForumPtr forumInfo);
	virtual QVariant doGetUnreadForums();

	virtual QVariant doGetPostList(ThreadPtr t, PostListOptions listOption, int webOptions) override;

private:
	void loadConfig();

	QString getRequestXml(const QString&, ParamList = ParamList());
    const QString uploadString(const QString& payload);

	void walkForum(QVariant* variant);

	ForumPtr makeForumObject(QVariant* variant);
	ThreadPtr makeThreadObject(QVariant* variant);
	PostPtr makePostObject(QVariant* variant);

	void getRootId(QString data);
	QString getForumName();	

	virtual QVariant doPostList(ThreadPtr threadInfo, int options);
	virtual QVariant doPostList1(ThreadPtr threadInfo, int options);

    WebClient              _webclient;

	QString					_rootId;
	bool					_rootIdRealized;
	bool					_configLoaded;

    QString					_version;
	bool					_bUseMD5;
	bool					_bUseSha1;
    int						_apiLevel;

    QString					_forumName;	// so we don't make multiple calls
	ForumMap				_forumMap;
	bool					_forumMapInitialized;

    // Bug #117: We need to track the last login and then do a login every 15 minutes.
    LoginInfo               _loginInfo;
    QDateTime               _lastLogin;

	QMutex					_mutex;
};

} // namespace
