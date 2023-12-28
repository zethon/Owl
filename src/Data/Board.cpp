// Owl - www.owlclient.com
// Copyright (c) 2012-2023, Adalid Claure <aclaure@gmail.com>

#include <boost/functional/hash.hpp>

#include <Utils/Settings.h>
#include <Utils/OwlLogger.h>
#include <Utils/OwlUtils.h>

#include "Board.h"

namespace owl
{

const char* const Board::Options::USE_USERAGENT			= "web.customagent.enabled";
const char* const Board::Options::USERAGENT				= "web.customagent.value";
const char* const Board::Options::USE_ENCRYPTION		= "encryption.enabled";;
const char* const Board::Options::ENCSEED				= "encryption.seed";;
const char* const Board::Options::ENCKEY				= "encryption.key";

Board::Board(const QString& url)
    : _url(url),
    _bEnabled(true),
    _bAutoLogin(false),
    _status(BoardStatus::OFFLINE),
    _options(StringMapPtr(new StringMap())),
    _logger(owl::initializeLogger("Board"))
{}

Board::Board()
	: Board(QString())
{
    // nothing to do
}	

QString getAbbreviatedName(const QString& name)
{
    QString strTemp(name);
    int iCount = strTemp.count("-");

    if (iCount > 1)
    {
        strTemp = strTemp.mid(0, strTemp.indexOf("-"));
        strTemp = strTemp.simplified();
    }

    if (strTemp.length() > 15 && strTemp.count(" ") > 0)
    {
        QStringList list(strTemp.split(" "));
        strTemp.clear();

        for (QString s : list)
        {
            if (!s.isEmpty())
            {
                strTemp.append(s.at(0));
            }
        }
    }

    return strTemp;
}

void Board::setParser(ParserBasePtr parser)
{
	if (parser == _parser)
	{
		return;
	}

	if (_parser != nullptr)
	{
		// tell the old parser object to stop sending us signals
		_parser->disconnect(this);

        // release the old object
        _parser.reset();
	}

	// store the parser object
	_parser = parser;

	if (_parser != nullptr)
	{
		setProtocolName(_parser->getName());

		_parser->setUserAgent(getUserAgent());

		QObject::connect(_parser.get(), SIGNAL(boardwareInfoCompleted(StringMap)), 
			this, SLOT(boardwareInfoEvent(StringMap)), Qt::DirectConnection);

		QObject::connect(_parser.get(), SIGNAL(loginCompleted(StringMap)), 
			this, SLOT(loginEvent(StringMap)), Qt::DirectConnection);

		QObject::connect(_parser.get(), SIGNAL(forumListCompleted(ForumList)),
			this, SLOT(forumListEvent(ForumList)), Qt::DirectConnection);

		QObject::connect(_parser.get(), SIGNAL(getForumCompleted(ForumPtr)),
			this, SLOT(getForumEvent(ForumPtr)), Qt::DirectConnection);

		QObject::connect(_parser.get(), SIGNAL(getThreadsCompleted(ForumPtr)),
			this, SLOT(getThreadListEvent(ForumPtr)), Qt::DirectConnection);

		QObject::connect(_parser.get(), SIGNAL(getPostsCompleted(ThreadPtr)),
			this, SLOT(getPostsEvent(ThreadPtr)), Qt::DirectConnection);

		QObject::connect(_parser.get(), SIGNAL(markForumReadCompleted(ForumPtr)),
			this, SLOT(markForumReadEvent(ForumPtr)), Qt::DirectConnection);

        QObject::connect(_parser.get(), SIGNAL(errorNotification(const Exception&)),
            this, SIGNAL(onRequestError(const Exception&)), Qt::DirectConnection);
	}
}

void Board::setLoginInfo(const LoginInfo& var)
{
	setUsername(var.first);
	setPassword(var.second);
}

void Board::setCustomUserAgent(bool bCustom)
{
    getOptions()->setOrAdd(Options::USE_USERAGENT, static_cast<bool>(bCustom));
}

bool Board::getCustomUserAgent() const
{
    return getOptions()->getBool(Options::USE_USERAGENT);
}

void Board::setUserAgent(const QString &var)
{
    getOptions()->setOrAdd(Options::USERAGENT, static_cast<QString>(var));
    _parser->setUserAgent(var);
}

QString Board::getUserAgent() const
{
    return getOptions()->getText(Options::USERAGENT, false);
}

void Board::boardwareInfoEvent(StringMap params)
{
    Q_EMIT onBoardwareInfo(shared_from_this(), params);
}

void Board::login()
{
	LoginInfo info(getUsername(), getPassword());

	getParser()->loginAsync(info);
}

void Board::loginEvent(StringMap params)
{
	if (params.getBool("success"))
	{
        _status = BoardStatus::ONLINE;
	}

	Q_EMIT onLogin(shared_from_this(), params);
}

void Board::submitNewThread(ThreadPtr thread)
{
    getParser()->submitNewThreadAsync(thread);
}

void Board::submitNewPost(PostPtr postInfo)
{
	getParser()->submitNewPostAsync(postInfo);
}

void Board::submitNewItem(ThreadPtr thread)
{
    getParser()->submitNewThreadAsync(thread);
}

void Board::submitNewItem(PostPtr postInfo)
{
    getParser()->submitNewPostAsync(postInfo);
}

void Board::newThreadEvent(ThreadPtr thread)
{
	// TODO: do more to this thread?
    Q_EMIT onNewThread(shared_from_this(), thread);
}

void Board::newPostEvent(PostPtr post)
{
	if (post)
	{
        post->setBoard(shared_from_this());
	}

    Q_EMIT onNewPost(shared_from_this(), post);
}
    
void Board::forumListEvent(ForumList list)
{
    for (ForumPtr f : list)
	{
        f->setBoard(shared_from_this());

		if (!_forumHash.contains(f->getId()))
		{
			f->setBoard(shared_from_this());
			_forumHash.insert(f->getId(), f);
		}
	}

	Q_EMIT onForumList(shared_from_this(), list);
}
    
void Board::requestThreadList(ForumPtr forum)
{
    requestThreadList(forum, ParserEnums::REQUEST_DEFAULT);
}

void Board::requestThreadList(ForumPtr forum, int options)
{
	this->setCurrentForum(forum);
    int iPerPage = this->getOptions()->get<std::int32_t>("threadsPerPage");
    forum->setPerPage(iPerPage);
    
	getParser()->getThreadListAsync(forum, options);
}

void Board::requestPostList(ThreadPtr thread)
{
    requestPostList(thread, ParserEnums::REQUEST_DEFAULT);
}
    
void Board::requestPostList(ThreadPtr thread, int options, bool bForceGoto/*=false*/)
{
	this->setCurrentThread(thread);
    int iPerPage = this->getOptions()->get<std::int32_t>("postsPerPage");
    thread->setPerPage(iPerPage);

	if (!bForceGoto)
	{
        const auto viewOption =
            static_cast<ParserBase::PostListOptions>(SettingsObject().read("view.threads.action").toInt());

		getParser()->getPostsAsync(thread, viewOption, options);
	}
	else
	{
		getParser()->getPostsAsync(thread, ParserBase::PostListOptions::FIRST_POST, options);
	}
}

void Board::markForumRead(ForumPtr forum)
{
    getParser()->markForumReadAsync(forum);
}
    
/// Raised from the Parser object
/*
	getForumEvent is raised from the Parser object
	ForumPtr contains the forumId of the parent (the 
	forum that was requested) and the children list
	to be populated with sub forums. 
	This method is shallow and does not populate the
	threads

    \param ForumPtr	Reference to the pointer object constructed by the Parser object
		this
*/
void Board::getForumEvent(ForumPtr forum)
{
	// see if the parent forum has a hash reference
	ForumHash::iterator parent = _forumHash.find(forum->getId());
	if (parent != _forumHash.end())
	{
		ForumPtr oldParent = _forumHash.value(forum->getId());

		forum->setModelItem(oldParent->getModelItem());
		_forumHash.erase(parent);
	}

	// go through each sub forum and see if there's a hash reference
    for (ForumPtr f : forum->getForums())
	{
		f->setBoard(shared_from_this());
		forum->addChild(f);

		ForumHash::iterator pos = _forumHash.find(f->getId());
		if (pos != _forumHash.end())
		{
			ForumPtr oldForum = _forumHash.value(f->getId());

			f->setModelItem(oldForum->getModelItem());
			_forumHash.erase(pos);
		}

		_forumHash.insert(f->getId(), f);
	}

	Q_EMIT onGetForum(shared_from_this(), forum);
}

void Board::getThreadListEvent(ForumPtr forum)
{
	if (forum != nullptr)
	{
        auto sharedFromThis = shared_from_this();
		ForumPtr current = this->getCurrentForum();

		if (current->getId() == forum->getId())
		{
            // since the parser has no concept of a Board,
            // go through each thread and set the board
            for (auto t : forum->getThreads())
            {
                t->setBoard(sharedFromThis);
            }
            
            Q_EMIT onGetThreads(sharedFromThis, forum);
		}
        else
        {
            _logger->warn("Board::getThreadListEvent() invoked with non-matching forumIds");
            Q_EMIT onGetThreads(sharedFromThis, forum);
        }
	}
    else
	{
        _logger->error("Bad forum passed to getThreadListEvent()");
	}
}

void Board::getPostsEvent(ThreadPtr thread)
{
	if (thread != nullptr)
	{
		ThreadPtr current = this->getCurrentThread();

		if (current->getId() == thread->getId())
		{
			auto sharedFromThis = shared_from_this();

			// since the parser has no concept of a board, the board 
			// goes through and sets the board object of each post, 
			// which seems.. silly
			for (auto p : thread->getPosts())
			{
				p->setBoard(sharedFromThis);
			}

			Q_EMIT onGetPosts(shared_from_this(), thread);
		}
	}
    else
	{
        _logger->error("Bad thread passed to getPostsEvent()");
	}
}

void Board::markForumReadEvent(ForumPtr f)
{
    Q_EMIT onMarkedForumRead(shared_from_this(), f);
}

void Board::crawlSubForum(ForumPtr parent, ForumIdList* dupList /*= nullptr*/, bool bThrow /*= true*/)
{
	Q_ASSERT(!parent->getId().isEmpty());

	ForumList forums = _parser->getForumList(parent->getId());
	parent->getForums().clear();

    for (ForumPtr& forum : forums)
	{
		forum->setBoard(shared_from_this());
		parent->addChild(forum);
		parent->getForums().push_back(forum);

        if (dupList != nullptr
			&& !dupList->contains(forum->getId())
			&& forum->getForumType() != owl::Forum::LINK)
		{
            _logger->debug("Crawling Sub Forum '{}' ({})",
                forum->getName().toStdString(), forum->getId().toStdString());

			dupList->push_back(forum->getId());
			crawlSubForum(forum, dupList, bThrow);
		}
	}
}

void Board::crawlRoot(bool bThrow /*= true*/)
{
	ForumIdList dupList;

	_root.reset(); // release the root
	_forumHash.clear();

	try
	{
		_root = Forum::createRootForum(_parser->getRootForumId());
		ForumList list = _parser->getForumList(_root->getId());

		for(ForumPtr& forum : list)
		{
			try
			{
				_root->getForums().push_back(forum);
				_root->addChild(forum);
				forum->setBoard(shared_from_this());

				_forumHash.insert(forum->getId(), forum);

				if (forum->getForumType() != owl::Forum::LINK)
				{
                    _logger->debug("Crawling Root Forum: {} ({})",
                        forum->getName().toStdString(), forum->getId().toStdString());

					crawlSubForum(forum, &dupList, bThrow);
				}

				// even if the this is a Forum::LINK, add it to the duplicate list
				// to be doule sure we don't crawl it
				if (!dupList.contains(forum->getId()))
				{
					dupList.push_back(forum->getId());
				}
			}
			catch (const owl::Exception& e)
			{
				if (bThrow)
				{
					throw;
				}
				else
				{
                    _logger->warn("Parser error:'{}'", e.message().toStdString());
				}
			}
		}

	}
	catch (const owl::Exception& e)
	{
		_root.reset();
        _logger->error("Parsing exception while crawling '{}': {}",
            _url.toStdString(), e.message().toStdString());

		if (bThrow)
		{
			throw;
		}
	}	
}

ForumPtr Board::getRootStructure(bool bThrow /* = true */)
{
    ForumIdList dupList;
    ForumPtr root = Forum::createRootForum();

    if (!_parser) return root;

	try
	{
        ForumList list = _parser->getForumList(root->getId());
        for (ForumPtr& forum : list)
		{
			try
			{
				root->getForums().push_back(forum);
				root->addChild(forum);
				forum->setBoard(shared_from_this());
                
				if (forum->getForumType() != owl::Forum::LINK)
				{
                    _logger->debug("Crawling Root Forum: {} ({})",
                        forum->getName().toStdString(), forum->getId().toStdString());

					crawlSubForum(forum, &dupList, bThrow);
				}
                
				// even if the this is a Forum::LINK, add it to the duplicate list
				// to be doule sure we don't crawl it
				if (!dupList.contains(forum->getId()))
				{
					dupList.push_back(forum->getId());
				}
			}
			catch (const owl::Exception& e)
			{
				if (bThrow)
				{
					throw;
				}
				else
				{
                    _logger->warn("Parser error:'{}'", e.message().toStdString());
				}
			}
		}
        
	}
	catch (const owl::Exception& e)
	{
        _logger->error("Parsing exception while crawling '{}': {}",
            _url.toStdString(), e.message().toStdString());
        
		if (bThrow)
		{
			throw;
		}
	}
    
    return root;
}

void Board::updateUnread()
{
    _logger->info("Updating unread threads for {}", this->getName().toStdString());
    
	try
	{
		updateForumHash();
		ForumList list = _parser->getUnreadForums();
        _hasUnread = list.size() > 0;
        Q_EMIT onGetUnreadForums(shared_from_this(), list);
	}
	catch (const owl::Exception& e)
	{
        _logger->error("Exception '{}'", e.message().toStdString());
	}

    _logger->info("Leaving updateUnread of {}", this->getName().toStdString());
}

/// Called when changes to the forum structre of the Board are made
/*
	To be thread safe, the mutex is locked in updateForumHash and
	the actual work is delegated to doUpdateHash(). _forumHash
	is used for O(N) lookup of ForumPtr object by forumId.

	\NOTE: This method does not (currently) remove references in the _forumHash
		that are not in the forum structure itself
*/
void Board::updateForumHash()
{
	QMutexLocker locker(&_hashMutex);
	doUpdateHash(this->getRoot());
}

QString Board::getName() const
{
    return _name;
}

QString Board::getServiceUrl() const
{
    if (_serviceUrl.trimmed().isEmpty())
    {
        return this->getUrl();
    }

    return _serviceUrl;
}

void Board::doUpdateHash(ForumPtr parent)
{
    if (!_forumHash.contains(parent->getId()))
    {
        _forumHash.insert(parent->getId(), parent);
    }

	ForumList childList = parent->getForums();
    for (ForumPtr& child : childList)
	{
		doUpdateHash(child);
	}
}

QIcon Board::convertIcon()
{
	// convert the 
	QByteArray buffer(getFavIcon().toLatin1());
	QImage image = QImage::fromData(QByteArray::fromBase64(buffer));

	if (image.width() < 24 || image.height() < 24)
	{
		image = image.scaled(24,24);
	}

	return QIcon(QPixmap::fromImage(image));
}

const BoardItemDocPtr Board::getBoardItemDocument()
{
    return _boardItemDoc;
}

void Board::setBoardItemDocument(BoardItemDocPtr doc)
{
    _boardItemDoc = doc;
}

QString Board::getPostQuote(PostPtr post)
{
    return _parser->getPostQuote(post);
}

void Board::refreshOptions()
{
	_parser->updateClients();
}
    
/**
 * @returns returns a copy of the Board's StringMap data
 */
StringMap Board::getBoardData()
{
    StringMap params;
    
    params.add("boardname", static_cast<QString>(this->getName()));
    params.add("username", static_cast<QString>(this->getUsername()));
    params.add("refreshRate", static_cast<int>(this->getOptions()->get<std::uint32_t>("refreshRate")));
    params.add("showImages", static_cast<bool>(this->getOptions()->getBool("showImages")));
    params.add("threadsPerPage", static_cast<int>(getOptions()->get<std::uint32_t>("threadsPerPage")));
    params.add("postsPerPage", static_cast<int>(getOptions()->get<std::uint32_t>("postsPerPage")));
    
    return params;
}

std::size_t Board::hash() const
{
    std::size_t seed = 0;
    boost::hash_combine(seed, _name.toStdString());
    boost::hash_combine(seed, _url.toStdString());
    boost::hash_combine(seed, _protocolName.toStdString());
    boost::hash_combine(seed, _uuid);
    return seed;
}

std::string Board::readableHash() const
{
    return fmt::format("{} ({})", this->getName().toStdString(), this->uuid());
}

///////////////////////////////////////////////////////////////////////////////
// BoardItemDoc
///////////////////////////////////////////////////////////////////////////////

BoardItemDoc::BoardItemDoc(const BoardPtr board, const QString& t)
    : _template(t)
{
    const uint boardIconWidth = 32;
    const uint boardIconHeight = 32;

    QByteArray buffer(board->getFavIcon().toLatin1());
	QImage image = QImage::fromData(QByteArray::fromBase64(buffer));

    qreal iXScale = static_cast<qreal>(boardIconWidth) / image.width();
    qreal iYScale = static_cast<qreal>(boardIconHeight) / image.height();

	// only scale the image if it's not the right size
    if (!owl::numericEquals(iXScale, 1.0) || !owl::numericEquals(iYScale, 1.0))
	{
		QTransform transform;
		transform.scale(iXScale, iYScale);
		image = image.transformed(transform, Qt::SmoothTransformation);
	}

	addResource(QTextDocument::ImageResource, QUrl("localdata://boardIcon.png"), QPixmap::fromImage(image));

    _dictionary.insert("%BOARDICON%", "localdata://boardIcon.png");
    _dictionary.insert("%BOARDNAME%", board->getName());
    _dictionary.insert("%BOARDURL%", board->getUrl());
    _dictionary.insert("%BOARDUSERNAME%", board->getUsername());
    _dictionary.insert("%BOARDSTATUSIMG%", ":/icons/offline.png");
    _dictionary.insert("%BOARDSTATUSALT%", tr("Offline"));

    reloadHtml();
}

void BoardItemDoc::addCSSItem(const QString& itemName, CSSProperties properties, bool bAppend)
{
	CSSProperties existingProps;
	
	if (_cssdoc.contains(itemName))
	{
		if (bAppend)
		{
			existingProps = _cssdoc[itemName];
		}
		else
		{ 
			_cssdoc.remove(itemName);
		}
	}

	// check the existing properties to see if we're adding anything 
	// that alreayd exists. if so, overwrite it with our new value
	if (existingProps.size() > 0)
	{
        const auto constKeys = properties.keys();
        for (const auto& key : constKeys)
		{
			if (existingProps.contains(key))
			{
				auto propVals = properties.value(key);
				existingProps[key] = propVals;
			}
			else
			{
				existingProps.insert(key, properties.value(key));
			}
		}
	}
	else
	{
		existingProps = properties;
	}

	// add these properties back to the CSS doc object
	_cssdoc.insert(itemName, existingProps);
}

void BoardItemDoc::addCSSItem(const QString& itemName, const QString& propKey, const QString& propVal, bool bAppend)
{
	CSSProperties props;
	props.insert(propKey, propVal);
	addCSSItem(itemName, props, bAppend);
}

void BoardItemDoc::setBackgroundColor(const QString& color)
{
	addCSSItem("body", "background-color", color);
	reloadStyleSheet();
}

void BoardItemDoc::reloadStyleSheet()
{
	QString css(getCSSText());
	addResource(QTextDocument::StyleSheetResource, QUrl("localdata:://format.css"), css);
}

QString BoardItemDoc::getCSSText() const
{
    QString retstr;
	QHashIterator<QString, CSSProperties> it(_cssdoc);

	while (it.hasNext())
	{
		it.next();

		auto itemName = it.key();
		auto properties = it.value();

		if (properties.size() > 0)
		{
			retstr.append(QString("%1 { ").arg(itemName));

            const auto constKeys = properties.keys();
            for (const auto& key : constKeys)
			{
				retstr.append(QString("%1:%2; ").arg(key, properties[key]));
			}

			retstr.append("}\n");
		}
	}

	return retstr;
}
    
void BoardItemDoc::reloadHtml()
{
    QString strHtml(_template);
    
    const auto constRef = _dictionary.keys();
    for (const auto& s : constRef)
    {
        strHtml = strHtml.replace(s, _dictionary[s]);
    }

    setHtml(strHtml);
}
    
void BoardItemDoc::setOrAddVar(const QString& k, const QString& v)
{
    if (_dictionary.contains(k))
    {
        _dictionary[k] = v;
    }
    else
    {
        _dictionary.insert(k,v);
    }    
}

} // namespace owl

std::size_t std::hash<owl::Board>::operator()(const owl::Board& board) const
{
    return board.hash();
}
