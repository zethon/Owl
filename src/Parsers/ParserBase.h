#pragma once
#include <QtCore>
#include "../Parsers/Forum.h"
#include "../Utils/WebClient.h"

namespace spdlog
{
    class logger;
}

namespace owl
{

struct ParserEnums
{
	enum RequestOptions
	{
		REQUEST_DEFAULT		= 0x0000,
		REQUEST_NOCACHE     = 0x0001,
		REQUEST_NOTIDY		= 0x0002
	};
};

typedef QVector<QString> ForumIdList;
typedef std::pair<QString, QString> LoginInfo;

class ParserBase;
using ParserBasePtr = std::shared_ptr<ParserBase>;

class ParserBase : 
	public QObject, 
    public std::enable_shared_from_this<ParserBase>
{
	Q_OBJECT

public:
	enum PostListOptions
	{
		FIRST_UNREAD = 0x0000,
		FIRST_POST = 0x0001,
		LAST_POST = 0x0002
	};

	ParserBase(const QString& name, const QString& prettyName, const QString& baseUrl);
	virtual ~ParserBase();

    virtual ParserBasePtr clone(ParserBasePtr other = ParserBasePtr())
    {
        if (!other)
        {
            OWL_THROW_EXCEPTION(OwlException("Cannot clone ParserBase"));
        }

        other->_name = _name;
        other->_description = _description;
        other->_baseUrl = _baseUrl;
        other->_userAgent = _userAgent;

        return other;
    }

    
	// gets the name of the parser passed in at construction
	void setParserName(const QString& name) { _name = name; }
	virtual QString getName() const { return _name; }
    virtual QString getPrettyName() const;

	virtual const QString getRootForumId() const { return "-1"; }

	// Configuration of the Parser
	// sets/gets the useragent string used for all http requests
	const QString& getUserAgent() const { return _userAgent; }
    void setUserAgent(const QString& agent);

    void addWatcher(WebClient* webClient);
    void removeWatcher(WebClient* webClient);

	// sets/gets the base address used for all queries
	const QString& getBaseUrl() const { return _baseUrl; }
	void setBaseUrl(const QString& var) { _baseUrl = var; }

    void setOptions(StringMapPtr var);

	void clearCache();
    
    WebClientConfig createWebClientConfig();
//	WebClientPtr createWebClient();

    // The first return value specifies the default number of posts per page for this parser. The
    // second value indicates if it is a read-only value or not. In most cases this will be 25,true,
    // except for the XenForo parser which is currently the only exception.
    virtual const std::pair<uint, bool> defaultPostsPerPage() const { return std::make_pair(25, false); }

    // This is the same as above except for threads per page instead of posts
    virtual const std::pair<uint, bool> defaultThreadsPerPage() const { return std::make_pair(25, false); }

	//****************************************************************************//
	// API
    virtual StringMap getBoardwareInfo();
	virtual void getBoardwareInfoAsync();

    virtual bool canParse(const QString&);

    virtual StringMap login(LoginInfo&);
	virtual void loginAsync(LoginInfo&);

    virtual StringMap logout(LoginInfo&);
	virtual void logoutAsync(LoginInfo&);

    virtual ForumList getRootSubForumList();

    virtual void getRootSubForumListAsync();

	virtual ForumList getForumList(const QString& id);
	virtual void getForumListAsync(const QString& id);

	virtual ForumList getUnreadForums();
	virtual void getUnreadForumsAsync();

	virtual ThreadList getThreadList(ForumPtr forumInfo);
	virtual ThreadList getThreadList(ForumPtr forumInfo, int options);
    virtual void getThreadListAsync(ForumPtr forumInfo);
	virtual void getThreadListAsync(ForumPtr forumInfo, int options);

	virtual PostList getPosts(ThreadPtr t, PostListOptions listOption, int webOptions = ParserEnums::REQUEST_DEFAULT);
	virtual void getPostsAsync(ThreadPtr t, PostListOptions listOptions, int webOptions = ParserEnums::REQUEST_DEFAULT);

    virtual void markForumRead(ForumPtr forumInfo);
    virtual void markForumReadAsync(ForumPtr forumInfo);

	virtual ThreadPtr submitNewThread(ThreadPtr threadInfo);
	virtual void submitNewThreadAsync(ThreadPtr threadInfo);

	virtual PostPtr submitNewPost(PostPtr postInfo);
	virtual void submitNewPostAsync(PostPtr postInfo);

	virtual QString getItemUrl(ForumPtr forum);
	virtual QString getItemUrl(ThreadPtr thread);
	virtual QString getItemUrl(PostPtr post);

	virtual QString getPostQuote(PostPtr post);

    virtual StringMap getEncryptionSettings();
	virtual void getEncryptionSettingsAsync();

	//****************************************************************************//

	virtual void getFavIconBuffer(QByteArray* buffer, const QStringList& iconFiles);
	virtual void updateClients();

    virtual QString getLastRequestUrl() = 0;

Q_SIGNALS:
    void loginCompleted(StringMap loginInfo);
	void logoutCompleted();
    void boardwareInfoCompleted(StringMap params);
	void forumListCompleted(ForumList list);

	void getUnreadForumsCompleted(ForumList list);
	void getForumCompleted(ForumPtr forum);
	void getThreadsCompleted(ForumPtr forum);
	void getPostsCompleted(ThreadPtr thread);
	void submitNewThreadCompleted(ThreadPtr thread);
	void submitNewPostCompleted(PostPtr post);
    void markForumReadCompleted(ForumPtr forum);
    void getEncryptionSettingsCompleted(StringMap settings);
	void errorNotification(OwlExceptionPtr ex);

private Q_SLOTS:
	void loginSlot();
	void logoutSlot();
	void boardInfoSlot();		
	void getUnreadForumsSlot();
	void getForumListSlot();
	void getThreadListSlot();	
	//void getPostListSlot();
	void getEncryptionSettingsSlot();
	void markForumReadSlot();
	
protected:

	////////////////////////////////////////////////////////////////
	// pure virtual methods

	virtual QVariant doLogin(const LoginInfo&) = 0;
	virtual QVariant doLogout() = 0;

	virtual QVariant doGetBoardwareInfo() = 0;
	virtual QVariant doTestParser(const QString&) = 0;
	
	virtual QVariant doGetForumList(const QString& forumId) = 0;
    virtual QVariant doThreadList(ForumPtr forumInfo, int options) = 0;

	//virtual QVariant doPostList(ThreadPtr threadInfo, int options) = 0;

	// virtual method called by getPosts() and getPostsAsync()
	virtual QVariant doGetPostList(ThreadPtr t, PostListOptions listOption, int webOptions)= 0;

	virtual QVariant doSubmitNewThread(ThreadPtr threadInfo) = 0;
	virtual QVariant doSubmitNewPost(PostPtr postInfo) = 0;
	virtual QVariant doGetEncryptionSettings() = 0;

	// /pure virtual methods
	////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////
	// virtual methods

	virtual QVariant doGetUnreadForums();
	virtual void getUnreadSubForums(ForumPtr f, ForumList* list);

    virtual QVariant doMarkForumRead(ForumPtr forumInfo);

	// /virtual methods
	////////////////////////////////////////////////////////////////

    StringMapPtr _options;

private:
    QString _name;
    QString _description; 

	QString _baseUrl;
	QString _userAgent;

    QList<WebClient*>			_clientWatchers;

	QFuture<QVariant>*			_requestFuture;
	QFutureWatcher<QVariant>*	_requestWatcher;
	
	QFuture<QVariant>			_future;
    QFutureWatcher<QVariant>	_watcher;

    std::shared_ptr<spdlog::logger>  _logger;
};

} // namespace owl

Q_DECLARE_METATYPE(owl::ParserBasePtr)
