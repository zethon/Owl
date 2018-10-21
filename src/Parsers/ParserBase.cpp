#include "ParserBase.h"

// ParserBase::getFavIcon() uses QNetwork* to get the favorite icon of
// the board, so it needs these.
// TODO: switch this over to the libcurl WebClient
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QtConcurrent>

namespace owl
{

ParserBase::ParserBase(const QString& name, const QString& prettyName, const QString& baseUrl)
    : _options(StringMapPtr(new StringMap())),
      _name(name),
	  _description(prettyName),
      _baseUrl(baseUrl),
	  _requestFuture(nullptr), 
      _requestWatcher(nullptr)
{
}	

ParserBase::~ParserBase()
{
}

QString ParserBase::getPrettyName() const
{
    if (!_description.isEmpty())
    {
        return _description;
    }

    return _name;
}

void ParserBase::setUserAgent(const QString &agent)
{
    if (!agent.trimmed().isEmpty())
    {
        _userAgent = agent;
        for (WebClient* client : _clientWatchers)
        {
            client->setUserAgent(_userAgent);
        }
    }
}

void ParserBase::addWatcher(WebClient *webClient)
{
    if (!_clientWatchers.contains(webClient))
    {
        _clientWatchers.push_back(webClient);
    }
}

void ParserBase::removeWatcher(WebClient *webClient)
{
    if (_clientWatchers.contains(webClient))
    {
        _clientWatchers.removeOne(webClient);
    }
}

void ParserBase::setOptions(StringMapPtr var)
{
    _options = var;
    updateClients();
}

bool ParserBase::canParse(const QString& html)
{
    bool bRetVal = false;

    QVariant variant = doTestParser(html);

    if (variant.canConvert<StringMap>())
    {
        StringMap results = variant.value<StringMap>();
		bRetVal = results.has("success") && results.getBool("success");
	}

	return bRetVal;
}

StringMap ParserBase::login(LoginInfo& info)
{
    return this->doLogin(info).value<StringMap>();
}

void ParserBase::loginAsync(LoginInfo& info)
{
	if (!_future.isRunning())
	{
		QObject::connect(&_watcher, SIGNAL(finished()), this, SLOT(loginSlot()));
		_future = QtConcurrent::run(std::function<QVariant(void)>(
			[this, info]
			{
				return doLogin(info);
			}));

	    _watcher.setFuture(_future);
	}
	else
	{
		logger()->debug("loginAsync: ParserBase::_future.isRunning() == true");
	}
}

void ParserBase::loginSlot()
{
    StringMap params;

	// TODO: make a generic templated slot-handler for these (see submitNewThreadSlot() too)
	// something like: void ParserBase<T>::futureSlot()
	QFutureWatcher<QVariant>* w = reinterpret_cast<QFutureWatcher<QVariant>* >(sender());
	if (w != nullptr)
	{
		try
		{
            _watcher.disconnect();
            params = w->future().result().value<StringMap>();
            Q_EMIT loginCompleted(params);
		}
		catch (const owl::OwlException& owe)
		{
            params.setOrAdd("success", (bool)false);
            params.setOrAdd("error", owe.details());
            logger()->warn("loginSlot() owl::OwlException: " + owe.details());
            Q_EMIT errorNotification(OwlExceptionPtr(owe.clone()));
		}
		catch (...)
		{
            const auto errorMessage = QString("There was an error connecting to ") + this->getBaseUrl() + QString(". Please check your login credentials, firewall/proxy settings or your Internet connection.");
            params.setOrAdd("success", (bool)false);
            params.setOrAdd("error", errorMessage);
            logger()->warn("loginSlot() error: " + errorMessage);
            Q_EMIT errorNotification(OwlExceptionPtr(new OwlException(errorMessage)));
        }
	}
	else
	{
        logger()->error("loginSlot() could not cast sender() to QFutureWatcher<StringMap>*");
        params.setOrAdd("success", (bool)false);
        params.setOrAdd("error", "There was an unknown login error");
        Q_EMIT errorNotification(OwlExceptionPtr(new OwlException("There was an unknown error.")));
	}	
}

StringMap ParserBase::logout(LoginInfo&)
{
    return this->doLogout().value<StringMap>();
}

void ParserBase::logoutAsync(LoginInfo&)
{
	if (!_future.isRunning())
	{
		connect(&_watcher, SIGNAL(finished()), this, SLOT(logoutSlot()));
		_future = QtConcurrent::run(this, &ParserBase::doLogout);
		_watcher.setFuture(_future);
	}
	else
	{
		logger()->debug("logoutAsync: ParserBase::_future.isRunning() == true");
	}
}

ForumList ParserBase::getRootSubForumList()
{
    return getForumList(getRootForumId());
}

void ParserBase::getRootSubForumListAsync()
{
    getForumListAsync(getRootForumId());
}

void ParserBase::logoutSlot()
{
    QFutureWatcher<QVariant>* w = reinterpret_cast<QFutureWatcher<QVariant>* >(sender());
	if (w != nullptr)
	{
		try
		{
            _watcher.disconnect();
            StringMap params = w->future().result().value<StringMap>();

			Q_EMIT logoutCompleted();
		}
		catch (const owl::OwlException& owe)
		{
			Q_EMIT errorNotification(OwlExceptionPtr(owe.clone()));
		}
	}
	else
	{
        logger()->error("logoutSlot() could not cast sender() to QFutureWatcher<StringMap>*");
        Q_EMIT errorNotification(OwlExceptionPtr(new OwlException("There was an unknown error.")));
	}	
}

StringMap ParserBase::getBoardwareInfo()
{
    return this->doGetBoardwareInfo().value<StringMap>();
}

void ParserBase::getBoardwareInfoAsync()
{
	if (!_future.isRunning())
	{
		connect(&_watcher, SIGNAL(finished()), this, SLOT(boardInfoSlot()));
		_future = QtConcurrent::run(this, &ParserBase::doGetBoardwareInfo);
		_watcher.setFuture(_future);
	}
	else
	{
		logger()->debug("getBoardwareInfoAsync: ParserBase::_future.isRunning() == true");
	}
}

void ParserBase::boardInfoSlot()
{

	QFutureWatcher<QVariant>* w = reinterpret_cast<QFutureWatcher<QVariant>* >(sender());
	if (w != nullptr)
	{
		try
		{
            _watcher.disconnect();
            StringMap params = w->future().result().value<StringMap>();

			Q_EMIT boardwareInfoCompleted(params);
		}
		catch (const owl::OwlException& owe)
		{
			Q_EMIT errorNotification(OwlExceptionPtr(owe.clone()));
			return;
		}
	}
	else
	{
        logger()->error("boardInfoSlot() could not cast sender() to QFutureWatcher<StringMap>*");
        Q_EMIT errorNotification(OwlExceptionPtr(new OwlException("There was an unknown error.")));
	}
}

ForumList ParserBase::getForumList(const QString& id)
{
	QVariant listVar(doGetForumList(id));

	if (listVar.canConvert<ForumList>())
	{
		return listVar.value<ForumList>();
	}

	return ForumList();
}

void ParserBase::getForumListAsync(const QString& id)
{
	if (!_future.isRunning())
	{
		connect(&_watcher, SIGNAL(finished()), this, SLOT(getForumListSlot()));
		_future = QtConcurrent::run(this, &ParserBase::doGetForumList, (const QString&)(id));
		_watcher.setFuture(_future);
	}
	else
	{
		logger()->debug("getForumListAsync: ParserBase::_future.isRunning() == true");
	}
}

void ParserBase::getForumListSlot()
{
	QFutureWatcher<QVariant>* w = reinterpret_cast<QFutureWatcher<QVariant>* >(sender());

	if (w != nullptr)
	{
		try
		{
            _watcher.disconnect();
			ForumList list = w->future().result().value<ForumList>();

			Q_EMIT forumListCompleted(list);
		}
		catch (const owl::OwlException& owe)
		{
			Q_EMIT errorNotification(OwlExceptionPtr(owe.clone()));
		}
		catch (...)
		{
            Q_EMIT errorNotification(OwlExceptionPtr(new OwlException("There was an unknown error.")));
		}
	}
	else
	{
		logger()->error("getForumListSlot() could not cast sender() to QFutureWatcher<QVariant>*");
        Q_EMIT errorNotification(OwlExceptionPtr(new OwlException("There was an unknown error.")));
	}	
}

ForumList ParserBase::getUnreadForums()
{
	QVariant listVar(doGetUnreadForums());

	if (listVar.canConvert<ForumList>())
	{
		return listVar.value<ForumList>();
	}

	return ForumList();
}

void ParserBase::getUnreadForumsAsync()
{
	if (!_future.isRunning())
	{
		connect(&_watcher, SIGNAL(finished()), this, SLOT(getUnreadForumsSlot()));
		_future = QtConcurrent::run(this, &ParserBase::doGetUnreadForums);
		_watcher.setFuture(_future);
	}
	else
	{
		logger()->debug("getUnreadForumsAsync: ParserBase::_future.isRunning() == true");
	}
}

void ParserBase::getUnreadForumsSlot()
{
	QFutureWatcher<QVariant>* w = reinterpret_cast<QFutureWatcher<QVariant>* >(sender());

	if (w != nullptr)
	{
		try
		{
            _watcher.disconnect();
			ForumList list = w->future().result().value<ForumList>();

			Q_EMIT getUnreadForumsCompleted(list);
		}
		catch (const owl::OwlException& owe)
		{
			Q_EMIT errorNotification(OwlExceptionPtr(owe.clone()));
		}
		catch (...)
		{
            Q_EMIT errorNotification(OwlExceptionPtr(new OwlException("There was an unknown error.")));
		}
	}
	else
	{
		logger()->error("getUnreadForumsSlot() could not cast sender() to QFutureWatcher<QVariant>*");
        Q_EMIT errorNotification(OwlExceptionPtr(new OwlException("There was an unknown error.")));
	}	
}
    
ThreadList ParserBase::getThreadList(ForumPtr forumInfo)
{
    return getThreadList(forumInfo, ParserEnums::REQUEST_DEFAULT);
}

ThreadList ParserBase::getThreadList(ForumPtr forumInfo, int options)
{
	QVariant var = doThreadList(forumInfo, options);
	ForumPtr forum = var.value<ForumPtr>();

	return forum->getThreads();
}

void ParserBase::getThreadListAsync(ForumPtr forumInfo)
{
    getThreadListAsync(forumInfo, ParserEnums::REQUEST_DEFAULT);
}
    
void ParserBase::getThreadListAsync(ForumPtr forumInfo, int options)
{
	if (!_future.isFinished())
	{
        logger()->debug("ParserBase::getThreadListAsync(): _future.isFinished() is FALSE, cancelling.");

        _watcher.disconnect();
        _future.cancel();
    }

    connect(&_watcher, SIGNAL(finished()), this, SLOT(getThreadListSlot()));
    _future = QtConcurrent::run(this, &ParserBase::doThreadList, (ForumPtr)(forumInfo),(int)(options));
    _watcher.setFuture(_future);
}

void ParserBase::getThreadListSlot()
{
	QFutureWatcher<QVariant>* w = reinterpret_cast<QFutureWatcher<QVariant>* >(sender());

	if (w != nullptr)
	{
		try
		{
            _watcher.disconnect();
			ForumPtr forum = w->future().result().value<ForumPtr>();

			Q_EMIT getThreadsCompleted(forum);
		}
		catch (const owl::OwlException& owe)
		{
			Q_EMIT errorNotification(OwlExceptionPtr(owe.clone()));
			return;
		}
		catch (...)
		{
            Q_EMIT errorNotification(OwlExceptionPtr(new OwlException("There was an unknown error.")));
			return;
		}
	}
	else
	{
		logger()->error("getThreadListSlot() could not cast sender() to QFutureWatcher<QVariant>*");
        Q_EMIT errorNotification(OwlExceptionPtr(new OwlException("There was an unknown error.")));
	}	
}

PostList ParserBase::getPosts(ThreadPtr t, PostListOptions listOption, int webOptions)
{
	return doGetPostList(t, listOption, webOptions).value<ThreadPtr>()->getPosts();
}

void ParserBase::getPostsAsync(ThreadPtr t, PostListOptions listOption, int webOptions)
{
	if (!_future.isFinished())
	{
        logger()->debug("ParserBase::getPostsAsync(): _future.isFinished() is FALSE, cancelling.");
        
        _watcher.disconnect();
        _future.cancel();
    }
    
    connect(&_watcher, &QFutureWatcherBase::finished, [this]
    {
        try
        {
            _watcher.disconnect();
            ThreadPtr thread = this->_future.result().value<ThreadPtr>();

            Q_EMIT getPostsCompleted(thread);
        }
        catch (const owl::OwlException& owe)
        {
            Q_EMIT errorNotification(OwlExceptionPtr(owe.clone()));
        }
        catch (...)
        {
            Q_EMIT errorNotification(OwlExceptionPtr(new OwlException("There was an unknown error.")));
        }
    });

    _future = QtConcurrent::run(this, &ParserBase::doGetPostList, (ThreadPtr)(t), (PostListOptions)listOption, (int)webOptions);
    
    _watcher.setFuture(_future);
}
    
void ParserBase::markForumRead(ForumPtr forumInfo)
{
    doMarkForumRead(forumInfo);
}

void ParserBase::markForumReadAsync(ForumPtr forumInfo)
{
	if (!_future.isRunning())
	{
		connect(&_watcher, SIGNAL(finished()), this, SLOT(markForumReadSlot()));
		_future = QtConcurrent::run(this, &ParserBase::doMarkForumRead,(ForumPtr)(forumInfo));
		_watcher.setFuture(_future);
	}
	else
	{
		logger()->debug("markForumReadAsync: ParserBase::_future.isRunning() == true");
	}
}

void ParserBase::markForumReadSlot()
{
	QFutureWatcher<QVariant>* w = reinterpret_cast<QFutureWatcher<QVariant>* >(sender());

	if (w != nullptr)
	{
		try
		{
            _watcher.disconnect();
			ForumPtr forumPtr = w->future().result().value<ForumPtr>();

			Q_EMIT markForumReadCompleted(forumPtr);
		}
		catch (const owl::OwlException& owe)
		{
			Q_EMIT errorNotification(OwlExceptionPtr(owe.clone()));
		}
		catch (...)
		{
            Q_EMIT errorNotification(OwlExceptionPtr(new OwlException("There was an unknown error.")));
		}
	}
	else
	{
		logger()->error("markForumReadSlot() could not cast sender() to QFutureWatcher<QVariant>*");
        Q_EMIT errorNotification(OwlExceptionPtr(new OwlException("There was an unknown error.")));
	}	
}

ThreadPtr ParserBase::submitNewThread(ThreadPtr threadInfo)
{
	return doSubmitNewThread(threadInfo).value<ThreadPtr>();
}

void ParserBase::submitNewThreadAsync(ThreadPtr threadInfo)
{
    if (!_future.isRunning())
    {
        connect(&_watcher, &QFutureWatcherBase::finished, [this]
        {
            try
            {
                _watcher.disconnect();
                const auto thread = this->_future.result().value<ThreadPtr>();

                Q_EMIT submitNewThreadCompleted(thread);
            }
            catch (const owl::OwlException& owe)
            {
                Q_EMIT errorNotification(OwlExceptionPtr(owe.clone()));
            }
            catch (...)
            {
                Q_EMIT errorNotification(OwlExceptionPtr(new OwlException("There was an unknown error.")));
            }
        });

        _future = QtConcurrent::run(this, &ParserBase::doSubmitNewThread, (ThreadPtr)(threadInfo));
        _watcher.setFuture(_future);
    }
    else
    {
        logger()->debug("submitNewThreadAsync: ParserBase::_future.isRunning() == true");
    }
}

PostPtr ParserBase::submitNewPost(PostPtr postInfo)
{
	return doSubmitNewPost(postInfo).value<PostPtr>();
}

void ParserBase::submitNewPostAsync(PostPtr postInfo)
{
	if (!_future.isRunning())
	{
		connect(&_watcher, &QFutureWatcherBase::finished, [this]
		{
			try
			{
                _watcher.disconnect();
				PostPtr post = this->_future.result().value<PostPtr>();

				Q_EMIT submitNewPostCompleted(post);
			}
			catch (const owl::OwlException& owe)
			{
				Q_EMIT errorNotification(OwlExceptionPtr(owe.clone()));
			}
			catch (...)
			{
                Q_EMIT errorNotification(OwlExceptionPtr(new OwlException("There was an unknown error.")));
			}
		});

		_future = QtConcurrent::run(this, &ParserBase::doSubmitNewPost, (PostPtr)(postInfo));
		_watcher.setFuture(_future);
	}
	else
	{
		logger()->debug("submitNewPostAsync: ParserBase::_future.isRunning() == true");
	}
}

QString ParserBase::getItemUrl(ForumPtr forum)
{
	return QString();
}

QString ParserBase::getItemUrl(ThreadPtr thread)
{
	return QString();
}

QString ParserBase::getItemUrl(PostPtr post)
{
	return QString();
}

QString ParserBase::getPostQuote(PostPtr post)
{
	return QString("[QUOTE]%1[/QUOTE]\n\n").arg(post->getText());
}
    
/*******************************************************************************/

void ParserBase::getFavIconBuffer(QByteArray* buffer, const QStringList& iconFiles)
{
	bool bUseDefault = true;

	if (iconFiles.size() > 0)
	{
		QNetworkAccessManager manager;
		QUrl url(_baseUrl);

		for (auto iconFile : iconFiles)
		{
			if (!iconFile.startsWith("/"))
			{
				iconFile.prepend("/");
			}

			url.setPath(iconFile);

			if (url.isValid())
			{
				bool bDone = false;
				uint iCount = 0;
				do 
				{
					logger()->trace("Looking for favicon at URL: '%1'", url.toString());
					iCount++;

					QNetworkRequest request(url);

					// some servers return 406 if we don't set the User-Agent
					request.setRawHeader("User-Agent", QByteArray(getUserAgent().toLatin1()));

					QNetworkReply* pReply = manager.get(request);

					QEventLoop loop;
					connect(pReply, SIGNAL(finished()), &loop, SLOT(quit()), Qt::DirectConnection);
					loop.exec(QEventLoop::ExcludeUserInputEvents);

					QVariant statusCode = pReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
					int status = statusCode.toInt();

					if (pReply->error() == QNetworkReply::NoError && status == 200)
					{
						buffer->clear();
						buffer->append(pReply->readAll());
						QImage image = QImage::fromData(*buffer);
						if (!image.isNull())
						{
							logger()->trace("Found favicon at URL: '%1'", url.toString());
							bUseDefault = false;
							break;
						}

						bDone = true;
					}
					else if (status == 301)
					{
						QVariant locationVar = pReply->header(QNetworkRequest::LocationHeader);
						url = locationVar.toString();
					}

					// ensure that we don't redirect indefinitely
					if (iCount > 3)
					{
						bDone = true;
					}

				} while (!bDone);

				// we found a favicon, so stop searching
				if (!bUseDefault)
				{
					break;
				}
			}
			else
			{
				logger()->warn("Invalid 'board-defaults.iconFiles' setting '%1'", url.toDisplayString());
			}
		}
	}

	if (bUseDefault)
	{
		logger()->trace("Using default board icon for board '%1'", this->getBaseUrl());

		// load the default board icon
		QFile f(":/icons/board.png");
		buffer->clear();

		if (f.open(QIODevice::ReadOnly))
		{
			buffer->append(f.readAll());
		}
	}
}

QVariant ParserBase::doGetUnreadForums()
{
    ForumList retList;

    for (auto subForum : getRootSubForumList())
	{
		getUnreadSubForums(subForum, &retList);
	}

	return QVariant::fromValue(retList);
}

void ParserBase::getUnreadSubForums(ForumPtr parent, ForumList* pList)
{
	const int PAGENUM = 1;
	const int PERPAGE = 50;

    // unread threats aren't necessarily the first threads listed,
    // so we look at the first 50 threads. it is possible that old unread
    // threads will not show up if they are further down
	ForumList childList = getForumList(parent->getId());

	if (parent->hasUnread() || parent->getForumType() == Forum::CATEGORY)
	{
        ForumPtr info(new Forum(parent->getId()));
        info->setPageNumber(PAGENUM);
        info->setPerPage(PERPAGE);
        
		ThreadList threadList = getThreadList(info);

        for (auto thread : threadList)
		{
			if (thread->hasUnread())
			{
				pList->push_back(parent);
				break;
			}
		}

		// now search the children
        for (ForumPtr subChild : childList)
		{
			if (subChild->hasUnread() || subChild->getForumType() == Forum::CATEGORY)
			{
				getUnreadSubForums(subChild, pList);
			}
		}
	}
}

void ParserBase::updateClients()
{
	WebClientConfig config = createWebClientConfig();

    for (WebClient* client : _clientWatchers)
	{
		client->setConfig(config);
	}
}

WebClientConfig ParserBase::createWebClientConfig()
{
	WebClientConfig config;

	config.userAgent = getUserAgent();

	if (_options->getBool("encryption.enabled", false))
	{
		config.useEncryption = true;
		config.encryptKey = _options->getText("encryption.key");
		config.encryptSeed = _options->getText("encryption.seed");
	}
	else
	{
		config.useEncryption = false;
		config.encryptKey = "";
		config.encryptSeed = "";
	}

	return config;
}

StringMap ParserBase::getEncryptionSettings()
{
    return this->doGetEncryptionSettings().value<StringMap>();
}

void ParserBase::getEncryptionSettingsAsync()
{
	if (!_future.isRunning())
	{
		connect(&_watcher, SIGNAL(finished()), this, SLOT(getEncryptionSettingsSlot()));
		_future = QtConcurrent::run(this, &ParserBase::doGetEncryptionSettings);
		_watcher.setFuture(_future);
	}
	else
	{
		logger()->debug("getEncryptionSettingsAsync: ParserBase::_future.isRunning() == true");
	}
}

void ParserBase::getEncryptionSettingsSlot()
{
	QFutureWatcher<QVariant>* w = reinterpret_cast<QFutureWatcher<QVariant>* >(sender());
	if (w != nullptr)
	{
		try
		{
            _watcher.disconnect();
            StringMap params = w->future().result().value<StringMap>();

			Q_EMIT getEncryptionSettingsCompleted(params);
		}
		catch (const owl::OwlException& owe)
		{
			Q_EMIT errorNotification(OwlExceptionPtr(owe.clone()));
		}
	}
	else
	{
        logger()->error("getEncryptionSettingsSlot() could not cast sender() to QFutureWatcher<StringMap>*");
        Q_EMIT errorNotification(OwlExceptionPtr(new OwlException("There was an unknown error.")));
	}	
}

} // namespace owl
