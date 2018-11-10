#include <QtConcurrent>
#include <Parsers/ParserManager.h>
#include <Parsers/Tapatalk.h>
#include <Utils/OwlUtils.h>
#include <Utils/Settings.h>
#include "Data/BoardManager.h"
#include "ConfiguringBoardDlg.h"

#include <spdlog/spdlog.h>

#define DEFAULT_SHOWIMAGES              true
#define DEFAULT_AUTOLOGIN_ENABLED       true

// how often the board is checked for new posts (in seconds)
#define DEFAULT_POSTREFRESH_ENABLED     true
#define DEFAULT_POSTREFRESH_RATE        600

// how often the forum structure is checked (in days)
#define DEFAULT_TREEREFRESH_ENABLED     true
#define DEFAULT_TREEREFRESH_RATE        10

namespace owl
{

////////////////////////////////////////////////////////
// class methods
		
ConfiguringBoardDlg::ConfiguringBoardDlg(QWidget* parent)
	: QDialog(parent),
      _bCompleted(false),
	  _bSuccess(false),
	  _future(new QFuture<void>),
	  _watcher(new QFutureWatcher<void>),
      FORUMPATHS(QStringList { "", "forum", "forums", "community", "board", "messageboard" }),
      ICONFILES (QStringList { "/apple-touch-icon.png", "/favicon.ico" }),
      _logger { spdlog::get("Owl")->clone("ConfiguringBoardDlg") }
{
	setupUi(this);
    
    setFixedSize(this->width(),this->height());

	_workingMovie = new QMovie(":/icons/loading.gif", QByteArray(), this);
    movieLbl->setMovie(_workingMovie);
    _workingMovie->start();

    QObject::connect(this, SIGNAL(completeEvent(StringMap)), this, SLOT(onCompleted(StringMap)));

	submitErrorBtn->setVisible(false);
}

void ConfiguringBoardDlg::setInfo(const QString& url,
		const QString& username,
		const QString& password,
		const QString& parser,
		const bool asGuest)
{
	_urlString = url;
	_username = username;
	_password = password;
	_parser = parser;
	_asGuest = asGuest;

	_targetUrl = QUrl::fromUserInput(_urlString);
	_bAutoDetect = (_parser == "#AUTODETECT");
}

void ConfiguringBoardDlg::start()
{
	statusLbl->setText(QString("Connecting to %1").arg(_targetUrl.toString()));
	*_future = QtConcurrent::run(this, &ConfiguringBoardDlg::doConfigure);

	_watcher->setFuture(*_future);
	QObject::connect(_watcher, SIGNAL(finished()), this, SLOT(onConfigurationError()));
}

void ConfiguringBoardDlg::doConfigure()
{
    StringMap results;
    
	if (_bAutoDetect)
	{
        results = autoConfigure();
	}
    else
    {
		results = singleConfigure();        
    }
    
    Q_EMIT completeEvent(results);
    return;
}

void ConfiguringBoardDlg::onCompleted(StringMap params)
{
	_bCompleted = true;

	// TODO: IDEA -- StringMapException
    if (params.has("success"))
	{
        if (params.getBool("success"))
		{
			QPixmap icon;
			icon.load(":/icons/success_32.png");
			movieLbl->setPixmap(icon);

			statusLbl->setText("Board configured successfully!");
			_bSuccess = true;
            
            Q_EMIT newBoardAddedEvent(_newBoard);
            QDialog::accept();
		}
		else
		{
			QPixmap icon;
			icon.load(":/icons/error_32.png");
			movieLbl->setPixmap(icon);

			statusLbl->setStyleSheet("QLabel { color: red; }");

            // TODO: enable this button for future versions
            //submitErrorBtn->setVisible(true);

            if (params.has("statusCode"))
			{
				statusLbl->setText(
					QString(tr("Unable to connect to %1. Server returned an error code of %2"))
					.arg(_targetUrl.toString())
                    .arg(params.get<std::uint32_t>("statusCode")));
			}
			else
			{
                _errorStr = params.getText("msg", false);

				if (_errorStr.isEmpty())
				{
					_errorStr = tr("There was an unknown error configuring the new message board. Please try again.");
				}

				statusLbl->setText(_errorStr);
			}
		}
	}

	pushButton->setText("OK");
}

void ConfiguringBoardDlg::onClicked()
{
	if (!_bCompleted)
	{
		_future->cancel();
	}

	if (_bSuccess)
	{
		Q_EMIT newBoardAddedEvent(_newBoard);
		QDialog::accept();
	}
	else
	{
		QDialog::reject();
	}
}

owl::StringMap ConfiguringBoardDlg::autoConfigure()
{
//    const QStringList forumPaths { "", "forum", "forums", "community" };
	StringMap results;

	// assume failure!
	results.add("success", (bool)false); 
	results.add("msg", "No board could be found");

	QString	baseUrl(owl::sanitizeUrl(_targetUrl.toString()));
    _logger->info("Searching for board at: {}", baseUrl.toStdString());

	QString testUrl;
	bool bFound = false;
	bool bKeepTrying = true;

    QStringList protocols;
    protocols << "https" << "http";

	// try Tapatalk parser first
    for (const auto protocol : protocols)
    {
        for (QString path : FORUMPATHS)
        {
            QUrl tempUrl { baseUrl };
            tempUrl.setScheme(protocol);
            testUrl = tempUrl.toString();

            if (!path.isEmpty() && path != "/")
            {
                testUrl = testUrl + "/" + path;
            }

            _logger->debug("Trying parser {} at Url: {}", TAPATALK_NAME, testUrl.toStdString());

            // tapatalk is Owl's preferred parser, so try it first
            ParserBasePtr parser = PARSERMGR->createParser(TAPATALK_NAME, testUrl);

            try
            {
                StringMap info = parser->getBoardwareInfo();

                if (info.has("version") && info.getText("version").size() > 0)
                {
                    _logger->info("Board found at {} with parser {}",
                        testUrl.toStdString(), parser->getName().toStdString());

                    results = createBoard(parser->getName(), testUrl);
                    if (results.getBool("success"))
                    {
                        bFound = true;
                    }
                    else
                    {
                        // If we're here then we found Tapatalk but something
                        // else went wrong, like maybe the user may have entered
                        // a bad username or password
                        bKeepTrying = false;
                    }

                    // so break during the autoconfigure and display the
                    // error message in results
                    break;
                }
            }
            catch (const std::exception&)
            {
                // silenty capture failure since we might be trying bad Urls
                continue;
            }
        }

        if (bFound || !bKeepTrying)
        {
            break;
        }
    }

	if (!bFound && bKeepTrying)
	{
        QString html;
        HttpReplyPtr reply;

		// loop through each parser testing each url until we run 
		// out or find a compatible parser

        WebClient client;
		client.setThrowOnFail(false);

		QList<QString> parsers = ParserManager::instance()->getParserNames();

        for (const auto protocol : protocols)
        {
            for (QString path : FORUMPATHS)
            {
                reply.reset();
                html.clear();

                QUrl tempUrl { baseUrl };
                tempUrl.setScheme(protocol);

                if (!path.isEmpty() && path != "/")
                {
                    testUrl = testUrl + "/" + path;
                }

                try
                {
                    reply = client.GetUrl(testUrl, WebClient::NOTIDY | WebClient::NOCACHE);
                }
                catch (const std::exception&)
                {
                    // catch any errors from making the client.DownloadString() request
                    // since we could be trying a bad Url
                    continue;
                }

                if (reply && reply->data.size() > 0)
                {
                    html = reply->data;
                    for (QString parserName : parsers)
                    {
                        _logger->debug("Trying parser {} at Url: {}", parserName.toStdString(), testUrl.toStdString());
                        ParserBasePtr p = ParserManager::instance()->createParser(parserName, testUrl);

                        if (p->canParse(html))
                        {
                            _logger->info("Board found at '{}' with parser '{}'",
                                testUrl.toStdString(), p->getName().toStdString());

                            // settle up things like https vs http, and http://domain vs http://www.domain
                            testUrl = resolveFinalUrl(testUrl, reply->finalUrl);

                            results = createBoard(p->getName(), testUrl);
                            if (results.getBool("success"))
                            {
                                bFound = true;
                            }

                            // If we're here then we found a parser that can parse
                            // the board, but something else went wrong, like maybe
                            // the user may have entered a bad username or password
                            // so break during the autoconfigure and display the
                            // error message in results
                            break;
                        }
                    }

                    if (bFound)
                    {
                        break;
                    }
                }
            }

            if (bFound)
            {
                break;
            }
        }
    }

	return results;
}

owl::StringMap ConfiguringBoardDlg::createBoard(const QString& parserName, const QString& urlText)
{
    _logger->info("Configuring new board with parser {} at url {}",
        parserName.toStdString(), urlText.toStdString());

	StringMap resultParams;

	_newBoard = BoardPtr(new Board(urlText));

    ParserBasePtr parser = ParserManager::instance()->createParser(parserName, urlText);
    _newBoard->setParser(parser);

	owl::StringMapPtr options = _newBoard->getOptions();

    options->add("enableAutoRefresh", (bool)DEFAULT_POSTREFRESH_ENABLED);
    options->add("refreshRate", (uint)DEFAULT_POSTREFRESH_RATE);

    options->add("treeRefresh.enabled", (bool)DEFAULT_TREEREFRESH_ENABLED);
    options->add("treeRefresh.rate", (uint)DEFAULT_TREEREFRESH_RATE);

    options->add(Board::Options::USE_USERAGENT, (bool)false); // use the default user agent
    options->add(Board::Options::USERAGENT, SettingsObject().read("web.useragent").toString());

    const auto postsPP = parser->defaultPostsPerPage();
    options->add("postsPerPage", postsPP.first);

    const auto threadsPP = parser->defaultThreadsPerPage();
    options->add("threadsPerPage", threadsPP.first);

    options->add("showImages", DEFAULT_SHOWIMAGES);
    options->add("lastForumId", (int)-1);

	options->add("displayOrder", (uint)(BOARDMANAGER->getBoardCount() + 1));

	StringMap boardwareInfo = parser->getBoardwareInfo();

	statusLbl->setText(QString(tr("Detected %1 version %2. Attempting login."))
		.arg(boardwareInfo.getText("boardware"))
		.arg(boardwareInfo.getText("version")));

    _logger->info("Attempting login of new board");
	LoginInfo loginInfo(_username, _password);
	StringMap loginResult = parser->login(loginInfo);

	if (loginResult.getBool("success"))
	{
		QUrl tempUrl(urlText);

		if (boardwareInfo.has("name") && !boardwareInfo.getText("name").isEmpty())
		{
			_newBoard->setName(boardwareInfo.getText("name"));
		}
		else
		{
			_newBoard->setName(tempUrl.toDisplayString());
		}

		_newBoard->setUsername(_username);
		_newBoard->setPassword(_password);

        _newBoard->setAutoLogin(DEFAULT_AUTOLOGIN_ENABLED);
		_newBoard->setStatus(Board::ONLINE);

		statusLbl->setText(tr("Login successful. Retrieving forum list..."));
        _logger->info("Crawling new board '{}' ({})",
            _newBoard->getName().toStdString(), _newBoard->getUrl().toStdString());

		_newBoard->crawlRoot();

		if (_newBoard->getRoot()->getForums().size() > 0)
		{
			statusLbl->setText(tr("Retrieving message board icon..."));
            _logger->trace("Configuring '{}' -> retrieving message board icon", _newBoard->getName().toStdString());

			QByteArray buffer;
            parser->getFavIconBuffer(&buffer, ICONFILES);
			_newBoard->setFavIcon(buffer.toBase64());

			statusLbl->setText(tr("Looking for encryption settings..."));
            _logger->trace("Configuring '{}' -> looking for encryption settings", _newBoard->getName().toStdString());

			StringMap s = parser->getEncryptionSettings();
			if (s.has("success") && s.getBool("success"))
			{
				options->setOrAdd(Board::Options::USE_ENCRYPTION, (bool)true);
				options->setOrAdd(Board::Options::ENCKEY, (QString)s.getText("key"));
				options->setOrAdd(Board::Options::ENCSEED, (QString)s.getText("seed"));
			}
			else
			{
				options->add(Board::Options::USE_ENCRYPTION, (bool)false);
				options->add(Board::Options::ENCSEED, (QString)"");
				options->add(Board::Options::ENCKEY, (QString)"");
			}

			options->add("rootId", (QString)parser->getRootForumId());

            parser->setOptions(options);

			// save the parser we found
			_parser = parserName;

			// write the board info to the db
			BoardManager::instance()->createBoard(_newBoard);

            _logger->trace("Board saved. Name: '{}' Url: '{}' DBId: '{}'",
                _newBoard->getName().toStdString(), _newBoard->getUrl().toStdString(), _newBoard->getDBId());

			resultParams.add("success", true);
		}
		else
		{
            _logger->error("Unable to crawl the message board");

			resultParams.add("success", false);
			resultParams.add("msg", tr("The forum structure of the messageboard could not be determined."));
		}
	}
	else if (loginResult.has("error"))
	{
		resultParams.add("success", false);
		resultParams.add("msg", loginResult.getText("error"));
	}
	else
	{
		resultParams.add("success", false);
		resultParams.add("msg", tr("Connection to the message board could not be established. Either your username or your password is incorrect. Please check you typed them correctly and try again."));
	}

	return resultParams;
}

owl::StringMap ConfiguringBoardDlg::singleConfigure()
{
	StringMap results;

	// assume failure!
	results.add("success", (bool)false); 
	results.add("msg", "No board could be found");

    _logger->info("Searching for board at user select url '{}' and parser '{}'",
        _targetUrl.toString().toStdString(), _parser.toStdString());

    if (_parser.toLower() == "tapatalk4x")
    {
        results = manualTapatalkConfigure();
    }
    else
    {
        QString	baseUrl(owl::sanitizeUrl(_targetUrl.toString()));

        HttpReplyPtr reply;
        QString html;
        QString testUrl;
        bool bFound = false;

        WebClient client;
        client.setThrowOnFail(true);

        for (QString path : FORUMPATHS)
        {
            reply.reset();
            html.clear();
            testUrl = baseUrl;

            if (!path.isEmpty() && path != "/")
            {
                testUrl = testUrl + "/" + path;
            }

            auto parser = PARSERMGR->createParser(_parser, testUrl);

            try
            {
                _logger->debug("Trying Url: %1", testUrl.toStdString());
                reply = client.GetUrl(testUrl, WebClient::NOCACHE);
            }
            catch (const WebException&)
            {
                // catch any errors since we might be testing a bad url
                continue;
            }

            if (reply && reply->data.size() > 0)
            {
                html = reply->data;

                if (parser->canParse(html))
                {
                    _logger->info("Board found at '{}' with parser '{}'",
                        testUrl.toStdString(), parser->getName().toStdString());

                    {
                        QUrl testUrlObj = QUrl::fromUserInput(testUrl);
                        QUrl foundUrl = QUrl::fromUserInput(reply->finalUrl);

                        // if the user enters 'http' but the site is redirecting to 'https'
                        // we want to save the 'https'
                        if (testUrlObj.scheme().toLower() != foundUrl.scheme().toLower())
                        {
                            testUrlObj.setScheme(foundUrl.scheme());
                        }

                        // if the user enters something like http://juot.net but the requests
                        // keep getting redirect to http://www.juot.net, we want to know about
                        // that too
                        if (testUrlObj.host().toLower() != foundUrl.host().toLower())
                        {
                            testUrlObj.setHost(foundUrl.host());
                        }

                        testUrl = testUrlObj.toString();
                    }

                    results = createBoard(parser->getName(), testUrl);
                    if (results.getBool("success"))
                    {
                        bFound = true;
                    }

                    break;
                }
                else
                {
                    _logger->debug("No board found at '%1'", testUrl.toStdString());
                }
            }
        }
    }

    return results;
}

StringMap ConfiguringBoardDlg::manualTapatalkConfigure()
{
    StringMap results;

    results.add("success", (bool)false);
    results.add("msg", QString("No board could be found at '%1'").arg(_targetUrl.toString()));

    if (_targetUrl.fileName().compare("mobiquo.php", Qt::CaseInsensitive) == 0)
    {
        // the user has entered a full path to Tapatalk, so we will try that and only that
        QString testUrl = this->_targetUrl.toString();
        ParserBasePtr parser = PARSERMGR->createParser(_parser, testUrl);

        try
        {
            _logger->debug("Trying Url: {}", testUrl.toStdString());
            const auto info = parser->getBoardwareInfo();

            if (info.has("success"))
            {
                testUrl = resolveFinalUrl(testUrl, parser->getLastRequestUrl());
                results = createBoard(parser->getName(), testUrl);
            }
        }
        catch (const WebException& wx)
        {
            // catch any errors since we might be testing a bad url
            const QString error = QString("Could not request URL '%1' because: %2")
                .arg(testUrl)
                .arg(wx.details());

            _logger->debug(error.toStdString());
        }
    }
    else
    {        
        // a non-specific url, so we will do our usual forum path search for a tapatalk parser
        for (QString path : FORUMPATHS)
        {
            QString testUrl = QString("%1/%2")
                .arg(_targetUrl.toString())
                .arg(path);

            try
            {
                auto parser = PARSERMGR->createParser(_parser, testUrl);
                _logger->debug("Searching url '{}' with parser '{}'", testUrl.toStdString(), _parser.toStdString());
                const auto info = parser->getBoardwareInfo();

                if (info.has("success"))
                {
                    testUrl = resolveFinalUrl(testUrl, parser->getLastRequestUrl());
                    results = createBoard(parser->getName(), testUrl);
                    break;
                }
            }
            catch (const WebException& wx)
            {
                // catch any errors since we might be testing a bad url
                const QString error = QString("Could not request URL '%1' because: %2")
                    .arg(testUrl)
                    .arg(wx.details());

                _logger->debug(error.toStdString());
            }
        }
    }

    return results;
}

QString ConfiguringBoardDlg::resolveFinalUrl(const QString &testUrl, const QString &lastRequestUrl)
{
    QUrl testUrlObj = QUrl::fromUserInput(testUrl);
    QUrl foundUrl = QUrl::fromUserInput(lastRequestUrl);

    // if the user enters 'http' but the site is redirecting to 'https'
    // we want to save the 'https'
    if (testUrlObj.scheme().toLower() != foundUrl.scheme().toLower())
    {
        testUrlObj.setScheme(foundUrl.scheme());
    }

    // if the user enters something like http://juot.net but the requests
    // keep getting redirect to http://www.juot.net, we want to know about
    // that too
    if (testUrlObj.host().toLower() != foundUrl.host().toLower())
    {
        testUrlObj.setHost(foundUrl.host());
    }

    return testUrlObj.toString();
}

void ConfiguringBoardDlg::onConfigurationError()
{
	QFutureWatcher<void> *watcher = static_cast<QFutureWatcher<void> *>(sender());
	QFuture<void> future = watcher->future();

	try
	{
		// waitForFinished() will rethrow any exceptions that were thrown
		// in QtConcurrentRun
		future.waitForFinished();
	}
    catch (const OwlException& ex)
	{
		StringMap results;
		results.add("success", (bool)false); 
		results.add("msg", ex.message());

        Q_EMIT completeEvent(results);
	}
    catch (const QUnhandledException&)
	{
		StringMap results;
		results.add("success", (bool)false); 
		results.add("msg", QString("There was an unexpected error while configuring the board"));

        Q_EMIT completeEvent(results);
	}
}

} // end namespace
