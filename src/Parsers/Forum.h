// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <memory>
#include <vector>
#include <QtCore>
#include <QCryptographicHash>
#include <QtGui>
#include <log4qt/logger.h>
#include "../Utils/StringMap.h"

namespace owl
{

const static int PER_PAGE_DEFAULT = 20;

class Board;
typedef std::shared_ptr<Board> BoardPtr;
typedef std::weak_ptr<Board> BoardWeakPtr;

class BoardItem;
typedef std::shared_ptr<BoardItem> BoardItemPtr;
typedef QVector<owl::BoardItemPtr> BoardItemList;
typedef std::shared_ptr<BoardItemList> BoardItemListPtr;
    
class Post;
typedef std::shared_ptr<Post> PostPtr;
typedef QList<owl::PostPtr> PostList;

class Thread;
typedef std::shared_ptr<Thread> ThreadPtr;
typedef QList<owl::ThreadPtr> ThreadList;

class Forum;
typedef std::shared_ptr<owl::Forum> ForumPtr;
typedef QList<ForumPtr> ForumList;

struct DateTimeFormatOptions;

class TagList : public QStringList
{

public:
    operator QString() const
    {
        return this->join(",");
    }
};

class BoardItem : public QObject, public std::enable_shared_from_this<BoardItem>
{
	Q_OBJECT
	LOG4QT_DECLARE_QCLASS_LOGGER

public:

	BoardItem(const QString& id)
		: _id(id),
		  _bHasUnread(false),	  
		  _iPageNumber(1),
		  _iTotalPages(1),
		  _iPerPage(PER_PAGE_DEFAULT),
		  _childLock()
	{	  
		registerMeta();
	};

	// default constructor
	BoardItem() 
		: BoardItem(QString())
	{
	}

	// copy constructor
	BoardItem(const BoardItem& other)
	{
		_dbId = other._dbId;
		_id = other._id;
		_title = other._title;

		_lastUpdated = other._lastUpdated;
		_bHasUnread = other._bHasUnread;

		_owner = other._owner;

		if (_parent.lock())
		{
			_parent = other._parent;
		}

		for (BoardItemPtr bi : other._children)
		{
			BoardItemPtr nbi(new BoardItem(bi->getId()));
			_children.push_back(nbi);
		}

		_iPageNumber = other._iPageNumber;
		_iTotalPages = other._iTotalPages;
		_iPerPage = other._iPerPage;

		_vars = other._vars;
	}


    virtual ~BoardItem() = default;

    const QString getId() { return _id; }

	const int getDBId() { return _dbId;; }
	void setDBId(int var) { _dbId = var; }

	void setTitle(const QString& var) { _title = var; }
    const QString getTitle() const { return _title; }

    void setIconUrl(const QString& var) { _strIconUrl = var; }
    QString getIconUrl() const { return _strIconUrl; }

	BoardItemPtr getParent() const { return _parent.lock(); }
    virtual void setParent(BoardItemPtr parent)
    {
        _parent = parent;
    }

	void setBoard(const BoardPtr var) { _owner = var; }
    BoardWeakPtr getBoard() const { return _owner; }

	void setHasUnread(bool var) { _bHasUnread = var; }
	bool hasUnread() const { return _bHasUnread; }

	void setLastUpdated(const QDateTime& var) { _lastUpdated = var; }
	QDateTime getLastUpdated() { return _lastUpdated; }

	int getPageNumber() const { return _iPageNumber; }
	void setPageNumber(int var) { _iPageNumber = var; }

	int getPageCount() const { return _iTotalPages; }
	void setPageCount(int var) { _iTotalPages = var; }

	int getPerPage() const { return _iPerPage; }
	void setPerPage(int var) { _iPerPage = var; }

	template <typename NewTypePtrT>
	NewTypePtrT cast(BoardItemPtr b, bool throwOnFail)
	{
        if (b == nullptr && !throwOnFail)
		{
			return NewTypePtrT();
		}
        else if (b == nullptr)
		{
            OWL_THROW_EXCEPTION(OwlException("Cannot upcast board item, object is null"));
		}

        NewTypePtrT t =  std::dynamic_pointer_cast<typename NewTypePtrT::element_type, BoardItem>(b);

        if (t == nullptr && throwOnFail)
		{
            OWL_THROW_EXCEPTION(OwlException("Cannot upcast board item, object is not the correct type"));
		}

		return t;
	}

	template <typename NewTypePtrT>
	NewTypePtrT upCast(bool bThrowOnFail = true)
	{
		return cast<NewTypePtrT>(shared_from_this(), bThrowOnFail);
	}

	void setVar(const QString& key, const QString& value);
	QString getVar(const QString& key, bool bThrow = true);
	StringMap& getVars() { return _vars; }

    BoardItemPtr addChild(BoardItemPtr child, bool bThrow = true);

	void removeChild(BoardItemPtr child, bool bThrow = true);
	QList<BoardItemPtr>& getChildren() { return _children; }

	virtual bool operator==(BoardItem& other)
	{
		return getId() == other.getId();
	}

	virtual bool operator!=(BoardItem& other)
	{
		return !(*this == other);
	}

protected:
	virtual void registerMeta();

	int _dbId;
	QString _id;
	QString _title;
    QString     _strIconUrl;

	QDateTime _lastUpdated;
	bool _bHasUnread;

    BoardWeakPtr _owner;

    std::weak_ptr<BoardItem> _parent;
	QList<BoardItemPtr> _children;	

	int _iPageNumber;
	int _iTotalPages;
	int _iPerPage;
    
	QMutex _childLock;

private:

	StringMap	_vars;
};

class Post : public BoardItem
{

public:

	// constructor
	Post(const QString& id)
		: BoardItem(id)
	{
	};

	// default constructor
    Post() : Post(QString()) { }

    virtual ~Post() = default;

	void setAuthor(const QString& var) { _strAuthor = var; }
	const QString& getAuthor() const { return _strAuthor; }

	void setText(const QString& var) { _strText = var; }
	const QString& getText() const { return _strText; }
	QString toPlainText() 
	{
		return _strText.remove(QRegExp("<[^>]*>"));
	}
	
	void setIndex(int var) { _iIndex = var; }
	int getIndex() const { return _iIndex; }

	void setDatelineString(const QString& var) { _datelineStr = var; }
	const QString& getDatelineString() const { return _datelineStr; }

	void setDateTime(const QDateTime& var) { _dateline = var; }
	const QDateTime& getDateTime() const { return _dateline; }	

    QString getPrettyTimestamp(const DateTimeFormatOptions&);

private:
    QDateTime   _dateline;
    QString     _datelineStr;       // raw timestamp parsed from remote data
    QString     _prettyDateLine;    // cached pretty string so we don't create it more than once

    QString     _strAuthor;
    QString     _strText;

    int _iIndex = -1;
};

class Thread : public BoardItem
{
public:

	// default constructor
	Thread() { };
    
    Thread(const QString& id)
		: BoardItem(id),
		  _bSticky(false)
	{
		// do nothing
	};

    virtual ~Thread() = default;

    void setLastPost(std::shared_ptr<Post> var) { _lastPost = var; }
    std::shared_ptr<Post> getLastPost() const { return _lastPost; }

    void setFirstUnreadPost(std::shared_ptr<Post> var) { _firstUnread = var; }
    std::weak_ptr<Post> getFirstUnread() const { return _firstUnread; }
	QString getFirstUnreadId() const 
	{ 
		if (_firstUnread)
		{
			return _firstUnread->getId();
		}

		return QString();
	}

	void setViews(int var) { _iViews = var; }
	int getViews() const { return _iViews; }

    void setReplyCount(int var);
    int getReplyCount() const { return _iReplyCount; }

	void setAuthor(const QString& var) { _strAuthor = var; }
	const QString& getAuthor() const { return _strAuthor; }

	void setPreviewText(const QString& var) { _strPreviewText = var; }
    QString getPreviewText(uint maxLen = 128) const;

	void setSticky(bool var) { _bSticky = var; }
	bool isSticky() const { return _bSticky; }

    QList<std::shared_ptr<Post> >& getPosts() { return _posts; }

    void pushTag(const QString& tag) { _tags.push_back(tag); }
    TagList getTags() { return _tags; }

private:
    uint _iViews = 0;	// support?

    QString     _strAuthor;
    QString     _strPreviewText;
    TagList     _tags;
    bool        _bSticky;
    uint        _iReplyCount = 0;
	
    QList<std::shared_ptr<Post> > _posts;
    std::shared_ptr<Post>			_lastPost;
    std::shared_ptr<Post>			_firstUnread;
};

class Forum : public BoardItem 
{
public:
	typedef enum
	{ 
        UNKNOWN     = 0,
		FORUM		= 1, 
		CATEGORY	= 2, 
		LINK		= 3
	} ForumType;

    static std::shared_ptr<owl::Forum> createRootForum(QString rootId = "-1")
	{
        std::shared_ptr<owl::Forum> root(new Forum(rootId));
		root->setName("");
		root->setIsRoot(true);

		return root;
	}

    Forum() : BoardItem(QString("")) { }
	Forum(const QString& id);
    Forum(const QString& id, const QString& name);
	Forum(const QString& id, const QString& name, const ForumType type);

    virtual ~Forum();

	const QString& getName() const { return _name; }
	void setName(const QString& var) { _name = var; }

	void setModelItem(QStandardItem* item) { _modelItem = item; }
	QStandardItem* getModelItem() const { return _modelItem; }

    QList<std::shared_ptr<Forum> >& getForums() { return _forums; }
    QList<std::shared_ptr<Thread> >& getThreads() { return _threads; }

	void setForumType(const ForumType var) { _forumType = var; }
	ForumType getForumType() const { return _forumType; }
	QString getForumTypeString() const;

	void setIsRoot(bool bIsRoot) { _bIsRoot = bIsRoot; }
	bool IsRoot() const { return _bIsRoot; }
    
    virtual void setParent(BoardItemPtr parent) override
    {
        _parent = parent;
        
		auto parentForum = _parent.lock();
        if (parentForum && parentForum->upCast<ForumPtr>(false))
        {
			_iForumLevel = parentForum->upCast<ForumPtr>()->getLevel() + 1;
        }
    }
    
    void setDisplayOrder(int var) { _iDisplayOrder = var; }
    int getDisplayOrder() const { return _iDisplayOrder; }
    
    const uint getLevel() const { return _iForumLevel; }

	virtual bool isStructureEqual(Forum& other);
    virtual bool isStructureEqual(std::shared_ptr<Forum> other);

	virtual bool operator==(Forum& other)
	{
		return getId() == other.getId() 
			&& this->getName() == other.getName()
			&& getForumType() == other.getForumType();
	}

	virtual bool operator!=(Forum& other)
	{
		return !(*this == other);
	}

private:
    QString		_name;
	int			_iDisplayOrder;
	bool		_bIsRoot;
	ForumType	_forumType;
    
    // the level
    std::int32_t _iForumLevel = -1;
    
    QList<std::shared_ptr<Forum> >	_forums;
    QList<std::shared_ptr<Thread> >   _threads;

	QStandardItem* _modelItem;
};

} // namespace owl

Q_DECLARE_METATYPE(owl::PostPtr)
Q_DECLARE_METATYPE(owl::PostList)
Q_DECLARE_METATYPE(owl::ThreadPtr)
Q_DECLARE_METATYPE(owl::ThreadList)
Q_DECLARE_METATYPE(owl::ForumPtr)
Q_DECLARE_METATYPE(owl::ForumList)
