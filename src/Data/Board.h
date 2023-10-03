// Owl - www.owlclient.com
// Copyright (c) 2012-2019, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <vector>
#include <QtCore>
#include <QtGui>
#include <QSqlQuery>
#include <Parsers/ParserBase.h>
#include <Parsers/Forum.h>

namespace spdlog
{
    class logger;
}

namespace owl
{

class BoardItemDoc;
typedef std::shared_ptr<BoardItemDoc> BoardItemDocPtr;
typedef QHash<QString, ForumPtr> ForumHash;

enum class BoardStatus
{
    ERR,
    OFFLINE,
    ONLINE,
};

class Board :
    public QObject,
	public std::enable_shared_from_this<Board>
{
    Q_OBJECT

    Q_PROPERTY(std::string uuid READ uuid WRITE setUuid NOTIFY UuidChanged)

public:
	
	// used in the Board's options member for easy string names
	class Options
	{
		public:
			static const char* const USE_USERAGENT;
			static const char* const USERAGENT;
			
			static const char* const USE_ENCRYPTION;
			static const char* const ENCSEED;
			static const char* const ENCKEY;
	};

	Board();
    Board(const QString& url);

    virtual ~Board() = default;

    bool operator==(const Board& other) const
    {
        return hash() == other.hash();
    }

    bool operator!=(const Board& other) const
    {
        return !(*this == other);
    }

    // METHODS
	void login();

	// CRUD
    void submitNewThread(ThreadPtr);
	void submitNewPost(PostPtr);
    void submitNewItem(ThreadPtr);
    void submitNewItem(PostPtr);

	void crawlRoot(bool bThrow = true);	 // create forum structure
    ForumPtr getRootStructure(bool bThrow = true);
	void updateUnread(); // crawls the exists tree updating the forum's unread

    void requestThreadList(ForumPtr forum);
	void requestThreadList(ForumPtr forum, int options);
    
    void requestPostList(ThreadPtr thread);
	void requestPostList(ThreadPtr thread, int options, bool bForceGoto = false);
    
    void markForumRead(ForumPtr forum);

	// MAINTENANCE
	void updateForumHash();

	// PROPERTIES
	void setDBId(const uint id) { _boardId = id; }
	uint getDBId() const { return _boardId; }

    void setUuid(const std::string& var) { _uuid = var; }
    std::string uuid() const { return _uuid; }
    
    void setName(const QString& name) { _name = name; }
    QString getName() const;

    void setUrl(const QString& var) { _url = var; }
    QString getUrl() const { return _url; }
    
    void setServiceUrl(const QString& var) { _serviceUrl = var; }
    QString getServiceUrl() const;

	void setLoginInfo(const LoginInfo& var);
    
	void setUsername(const QString& var) { _username = var; }
    QString getUsername() const { return _username; }

	void setPassword(const QString& var) { _password = var; }
    QString getPassword() const { return _password; }

    void setCustomUserAgent(bool bCustom);
	
    bool getCustomUserAgent() const;

    void setUserAgent(const QString& var);

    QString getUserAgent() const;
    
    void setLastForumId(int id) { getOptions()->setOrAdd("lastForumId", static_cast<std::int32_t>(id)); }
    int getLastForumId() const { return getOptions()->get<std::int32_t>("lastForumId"); }

	void setEnabled(bool bEnabled) { _bEnabled = bEnabled; }
	bool isEnabled() const { return _bEnabled; }

	void setAutoLogin(bool var) { _bAutoLogin = var; }
	bool isAutoLogin() const { return _bAutoLogin; }

	void setParser(ParserBasePtr parser);
	ParserBasePtr getParser() const { return _parser; }

    void setProtocolName(const QString& var) { _protocolName = var; }
    QString getProtocolName() const { return _protocolName; }

	void setCurrentForum(const ForumPtr var) { _currentForum = var; }
	ForumPtr getCurrentForum() const { return _currentForum; }

	void setCurrentThread(const ThreadPtr var) { _currentThread = var; }
	ThreadPtr getCurrentThread() const { return _currentThread; }

	void setStatus(const BoardStatus& status) { _status = status; }
	BoardStatus getStatus() const { return _status; }

	void setLastUpdate(const QDateTime& date) { _lastUpdate = date; }
	const QDateTime& getLastUpdate() const { return _lastUpdate; }

	StringMapPtr getOptions() const { return _options; }

	ForumPtr getRoot() const { return _root; }
	void setRoot(ForumPtr root) { _root = root; }

	void setFavIcon(const QString& var) { _iconBuffer = var; }
	QString getFavIcon() const { return _iconBuffer; }
	QIcon convertIcon();

    const BoardItemDocPtr getBoardItemDocument();
    
    void setBoardItemDocument(BoardItemDocPtr doc);

	void setModelItem(QStandardItem* item) { _modelItem = item; }
	QStandardItem* getModelItem() const { return _modelItem; }

	const ForumHash& getForumHash() { return _forumHash; }

    QString getPostQuote(PostPtr post);

    virtual void refreshOptions();
    
    StringMap getBoardData();

    ParserBasePtr cloneParser() const { return getParser()->clone(); }

    std::size_t hash() const;
    std::string readableHash() const;

    bool hasUnread() const noexcept { return _hasUnread; }
    void setHasUnread(bool v) noexcept { _hasUnread = v; }

Q_SIGNALS:
    void UuidChanged();
	void onBoardwareInfo(BoardPtr, StringMap);
	void onLogin(BoardPtr, StringMap);
	void onForumList(BoardPtr, ForumList list);
	void onGetForum(BoardPtr, ForumPtr);
	void onGetThreads(BoardPtr, ForumPtr);
	void onGetPosts(BoardPtr, ThreadPtr);
	void onGetUnreadForums(BoardPtr, ForumList);
	void onNewThread(BoardPtr, ThreadPtr);
	void onNewPost(BoardPtr, PostPtr);
    void onMarkedForumRead(BoardPtr, ForumPtr);
    void onRequestError(const Exception&);

public Q_SLOTS:
    void newThreadEvent(ThreadPtr thread);
    void newPostEvent(PostPtr);

private Q_SLOTS:
	void boardwareInfoEvent(StringMap params);
	void loginEvent(StringMap params);
	void forumListEvent(ForumList list);
	void getForumEvent(ForumPtr forum);
	void getThreadListEvent(ForumPtr);
	void getPostsEvent(ThreadPtr thread);
    void markForumReadEvent(ForumPtr);

private:
    void crawlSubForum(ForumPtr parent, ForumIdList* dupList = nullptr, bool bThrow = true);
	void doUpdateHash(ForumPtr parent);

	uint			_boardId;
    std::string     _uuid;

	QString			_name;
	QString			_url;
	QString			_serviceUrl;    
	QString			_protocolName;

	bool			_bCustomUserAgent;
	QString			_userAgent;

	QString			_username;
	QString			_password;

	bool			_bEnabled;
	bool			_bAutoLogin;
    bool            _hasUnread = false;

	QString			_iconBuffer;
    BoardItemDocPtr _boardItemDoc;

	ForumPtr		_currentForum;
	ThreadPtr		_currentThread;

	QStandardItem*	_modelItem;

	ForumPtr		_root;
	ForumHash		_forumHash;

	ParserBasePtr	_parser;
	BoardStatus		_status;
	StringMapPtr    _options;

	QDateTime		_lastUpdate;
    int             _lastForumId = -1;

	QMutex			_hashMutex;
    QMutex          _itemDocMutex;

    std::shared_ptr<spdlog::logger>  _logger;
};
    
class BoardItemDoc : public QTextDocument
{

public:
    BoardItemDoc(const BoardPtr b, const QString& t);
	virtual ~BoardItemDoc() = default;

	typedef QHash<QString, QString> CSSProperties;
	typedef QHash<QString, CSSProperties> CSSDoc;
    
	QString getCSSText() const;

	void addCSSItem(const QString&, CSSProperties, bool bAppend = true);
	void addCSSItem(const QString&, const QString&, const QString&, bool bAppend = true);

	void setBackgroundColor(const QString& strColor);
    
    void reloadHtml();
	void reloadStyleSheet();
    
    void setOrAddVar(const QString& k, const QString& v);
    
private:
    CSSDoc                  _cssdoc;
    QMap<QString, QString>  _dictionary;
    QString                 _template;
};

using BoardPtr = std::shared_ptr<Board>;
using BoardList = std::vector<owl::BoardPtr>;

QString getAbbreviatedName(const QString& name);

} // namespace owl

namespace std
{

template <>
struct hash<owl::Board>
{
    typedef std::size_t             result_type;
    typedef const owl::Board & argument_type;
    std::size_t operator()(const owl::Board &) const;
};

} // namespace std

Q_DECLARE_METATYPE(owl::BoardPtr)
Q_DECLARE_METATYPE(owl::BoardWeakPtr)
Q_DECLARE_METATYPE(owl::BoardList)
