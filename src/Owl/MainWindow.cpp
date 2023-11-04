// Owl - www.owlclient.com
// Copyright (c) 2012-2019, Adalid Claure <aclaure@gmail.com>

#include <QDomElement>
#include <QQuickItem>
#include "qxmlstream.h"
#include <Utils/Exception.h>
#include <Utils/Settings.h>
#include <Utils/OwlUtils.h>
#include <Utils/OwlLogger.h>
#include "AboutDlg.h"
#include "EditBoardDlg.h"
#include "ErrorReportDlg.h"
#include "PreferencesDlg.h"
#include "QuickAddDlg.h"
#include "Core.h"
#include "MainWindow.h"
#include "BoardUpdateWorker.h"

#ifdef Q_OS_WIN
#include "windows.h"
#elif defined(Q_OS_MACOS)
#include <QtMac>
#endif

#ifdef Q_OS_WIN
    #define BOARDICONWIDGETWIDTH         70
    #define CENTRALWIDGETWIDTH          275
#elif defined(Q_OS_MAC)
    // #define BOARDICONWIDGETWIDTH         70
    // #define CENTRALWIDGETWIDTH          250
#else
    #define BOARDICONWIDGETWIDTH         70
    #define CENTRALWIDGETWIDTH          250
#endif

#if defined(Q_OS_MAC)
extern "C" void setupTitleBar(WId winId);
#elif defined (Q_OS_WIN)
extern "C" void setupTitleBar(WId winId);
extern "C" void setShowMenuText(WId winId, const char* text);
extern "C" bool handleWindowsEvent(const owl::MainWindow&, void*, long*);
#endif

namespace owl
{

void initializeTitleBar(owl::MainWindow* window)
{
#if defined(Q_OS_WIN)
    window->setWindowIcon(QIcon(":/icons/logo_64.png"));
    window->setWindowTitle(QStringLiteral(APP_NAME));
    setupTitleBar(window->winId());
#elif defined(Q_OS_MAC)
    window->setWindowTitle(QString{});
    setupTitleBar(window->winId());
#else
    window->setWindowIcon(QIcon(":/icons/logo_64.png"));
    window->setWindowTitle(QStringLiteral(APP_NAME));
#endif
}

//////////////////////////////////////////////////////////////////////////
// SplashScreen
//////////////////////////////////////////////////////////////////////////
SplashScreen::SplashScreen(const QPixmap & pixmap)
    : QSplashScreen(pixmap),
      _bDoCheck(true)
{
    QLabel* label = new QLabel(this);
    label->move(10, this->size().height() - 17);
    label->setText("version " OWL_VERSION);
    label->show();
}

///////////////////////////////////////////////////////////////////////////////
// MainWindow
///////////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(SplashScreen *splash, QWidget *parent)
    : QMainWindow(parent),
      _splash(splash),
      _imageOverlay{this},
      _logger(owl::initializeLogger("MainWindow"))
{
    setupUi(this);
    setDockNestingEnabled(true);

    initializeTitleBar(this);
    toggleOldControls(false);

    // this->boardIconDockWidget->setMaximumWidth(BOARDICONWIDGETWIDTH);
    // this->boardIconDockWidget->setMinimumWidth(BOARDICONWIDGETWIDTH);
    // this->centralWidget()->setMaximumWidth(CENTRALWIDGETWIDTH);
    // this->centralWidget()->setMinimumWidth(CENTRALWIDGETWIDTH);

    // TODO: move this to the OwlApplication class
    readWindowSettings();
    
    // initialize the dictionaries
    SPELLCHECKER->init();

    // TODO: the appToolbar has been made invisible for release 1.0 but maybe it will
    //		 come back for later versions
    //createToolbar();
    appToolBar->setVisible(false);
    
    // create a blank title bar for the post view dock
    // postViewDockWidget->setTitleBarWidget(new QWidget(postViewDockWidget));
    // boardIconDockWidget->setTitleBarWidget(new QWidget(boardIconDockWidget));
    
    QTimer::singleShot(0, this, SLOT(onLoaded()));
}

void MainWindow::onLoaded()
{
    loadBoards();

    createBoardPanel();
    createThreadPanel();
    createPostPanel();

//    threadLoadingImg->setMovie(new QMovie(":/images/loading_small.gif", QByteArray(), this));
//    threadLoadingImg->hide();

//    postsLoadingImg->setMovie(new QMovie(":/images/loading_small.gif", QByteArray(), this));
//    postsLoadingImg->hide();

    updateSelectedForum();
    updateSelectedThread();

    createLinkMessages();

    createMenus();
    createStatusBar();
    
    switch (_postsPanePosition)
    {
        case PANERIGHT:
        {
            this->_actions.postPaneRight->setChecked(true);
            this->_actions.postPaneBelow->setChecked(false);
            this->_actions.postPaneHidden->setChecked(false);
        }
        break;
            
        case PANEBOTTOM:
        {
            this->_actions.postPaneRight->setChecked(false);
            this->_actions.postPaneBelow->setChecked(true);
            this->_actions.postPaneHidden->setChecked(false);
        }
        break;
            
        case PANEHIDDEN:
        {
            this->_actions.postPaneRight->setChecked(false);
            this->_actions.postPaneBelow->setChecked(false);
            this->_actions.postPaneHidden->setChecked(true);
        }
        break;
            
        default:
            OWL_THROW_EXCEPTION(Exception("Unknown PostsPane position"));
    }

//    postsWebView->resetView();
    _bDoneLoading = true;
}

void MainWindow::loadBoards()
{
    // "New Board" toolbar button in the boardToolbar
    auto addBoard = boardToolbar->addAction(QIcon(":/icons/newboard.png"), tr("New Board"));
    QObject::connect(addBoard, SIGNAL(triggered()), this, SLOT(onNewBoard()));
    boardToolbar->addSeparator();
    
    if (BoardManager::instance()->getBoardCount() == 0)
    {
        qDebug() << "TODO: Show that there are no boards";
    }
    else
    {
        int iErrors = 0;

        const auto& list = BOARDMANAGER->getBoardList();
        for (const BoardPtr& b : list)
        {
            if (initBoard(b) && b->isAutoLogin())
            {
                _logger->debug("Starting automatic login for board '{}' with user '{}'",
                    b->readableHash(), b->getUsername().toStdString());

                b->login();
            }
            else
            {
                iErrors++;
            }
        }

        if (iErrors > 0)
        {
            QString msg(tr("Some boards could not be loaded."));
            QMainWindow::statusBar()->showMessage(msg, 5000); 
        }
    }

    QObject::connect(boardToolbar, SIGNAL(actionTriggered(QAction*)), this, SLOT(onBoardToolbarItemClicked(QAction*)));
    QObject::connect(boardToolbar, &QToolBar::visibilityChanged, [this](bool bVisible)
    {
        if (bVisible)
        {
            this->_actions.showBoardbar->setText(tr("Hide Board Toolbar"));
        }
        else
        {
            this->_actions.showBoardbar->setText(tr("Show Board Toolbar"));
        }
    });
}

bool MainWindow::initBoard(const BoardPtr& b)
{
    const uint boardIconWidth = 32;
    const uint boardIconHeight = 32;
    
    QString boardItemTemplate = owl::getResourceHtmlFile("boardItem.html");
    Q_ASSERT(!boardItemTemplate.isEmpty());
    
    bool ok = false;

    try
    {
        if (b->isEnabled())
        {
            ParserBasePtr parser = ParserManager::instance()->createParser(b->getProtocolName(), b->getServiceUrl());
            parser->setOptions(b->getOptions());

            b->setParser(parser);
            connectBoard(b);

            // add the board to the _boardToolBar
            QByteArray buffer(b->getFavIcon().toLatin1());
            QImage image = QImage::fromData(QByteArray::fromBase64(buffer));

            // calculate the scaling factor based on wanting a 32x32 image
            qreal iXScale = static_cast<qreal>(boardIconWidth) / static_cast<qreal>(image.width());
            qreal iYScale = static_cast<qreal>(boardIconHeight) / static_cast<qreal>(image.height());

            // only scale the image if it's not the right size
            if (owl::numericEquals<double>(iXScale, qreal(1.0)) || owl::numericEquals<double>(iYScale, qreal(1.0)))
            {
                QTransform transform;
                transform.scale(iXScale, iYScale);
                image = image.transformed(transform, Qt::SmoothTransformation);
            }

            QIcon icon(QPixmap::fromImage(image));
            QString toolTip = QString("%1@%2").arg(b->getUsername()).arg(b->getName());

            // set the toolbar action
            QAction* boardAction = new QAction(boardToolbar);
            boardAction->setText(b->getName());
            boardAction->setIcon(icon);
            boardAction->setIconText(owl::getAbbreviatedName(b->getName()));
            boardAction->setToolTip(toolTip);
            boardAction->setData(QVariant::fromValue(BoardWeakPtr(b)));

            BoardMenu* boardMenu = new BoardMenu(BoardWeakPtr(b), this);
            QObject::connect(boardMenu, &BoardMenu::boardInfoSaved,
                [this](BoardPtr b, StringMap)
                {
                    _logger->trace("onBoardInfoSaved({}:{})",
                        b->getDBId(), b->getName().toStdString());

                    auto doc = b->getBoardItemDocument();
                    doc->setOrAddVar("%BOARDNAME%", b->getName());
                    doc->setOrAddVar("%BOARDUSERNAME%", b->getUsername());
                    doc->reloadHtml();

                    // search the toolbar (top of the client) and update the text
                    for (QAction* a : boardToolbar->actions())
                    {
                        if (b == a->data().value<BoardWeakPtr>().lock())
                        {
                            a->setText(b->getName());
                            a->setIconText(owl::getAbbreviatedName(b->getName()));
                            QString toolTip = QString("%1@%2").arg(b->getUsername()).arg(b->getName());
                            a->setToolTip(toolTip);
                            break;
                        }
                    }
                });

            boardAction->setMenu(boardMenu);
            boardToolbar->addAction(boardAction);

            // lastly start thr worker thread
            _workerMap.insert(b->hash(), new QThreadEx());

            ok = true;
        }
    }
    catch (const owl::Exception& ex)
    {
        b->setStatus(BoardStatus::ERR);

        _logger->warn("Failed to create parser of type '{}' for board '{}': {}",
            b->getProtocolName().toStdString(), b->getName().toStdString(), ex.message().toStdString());
    }

    return ok;
}

void MainWindow::openPreferences()
{
    using BoardEditAction = PreferencesDlg::BoardEditAction;
    PreferencesDlg dlg(this);

    QObject::connect(&dlg, &PreferencesDlg::onBoardEdit,
        [this](const BoardPtr board, BoardEditAction action)
        {
            Q_ASSERT(board != nullptr);
            if (action == BoardEditAction::Delete)
            {
               onBoardDelete(board);
            }
            else if (action == BoardEditAction::MoveUp)
            {
                onDisplayOrderChanged(board, -1);
            }
            else if (action == BoardEditAction::MoveDown)
            {
                onDisplayOrderChanged(board, 1);
            }
        });

//    QObject::connect(&dlg, &PreferencesDlg::reloadThreadPanel, this,
//        [this]()
//        {
//            this->threadListWidget->reload();
//        }, Qt::DirectConnection);

//    QObject::connect(&dlg, &PreferencesDlg::reloadPostPanel, this,
//        [this]()
//        {
//            this->postsWebView->reloadView();
//        }, Qt::DirectConnection);

    dlg.exec();
}

bool MainWindow::event(QEvent *event)
{
    QMainWindow::event(event);
    if (event->type() == QEvent::Show && !_bInitialized)
    {
        // servicePaneVisibility
        _bInitialized = true;

        auto timer = new QTimer(this);
        timer->setSingleShot(true);

        connect(timer, &QTimer::timeout, [=]()
        {
            if (this->boardToolbar->isVisible())
            {
                _actions.showBoardbar->setText("Hide Boards Toolbar");
            }
            else
            {
                _actions.showBoardbar->setText("Show Boards Toolbar");
            }
        });

        timer->start(0);
    }

    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    _logger->debug("Closing Owl window");
    writeWindowSettings();
    QMainWindow::closeEvent(event);
}

bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG")
    {
        return handleWindowsEvent(*this, message, result);
    }
#endif

    return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::onBoardToolbarItemClicked(QAction* action)
{
    if (!action->data().canConvert<BoardWeakPtr>())
    {
        return;
    }

    BoardPtr board = action->data().value<BoardWeakPtr>().lock();
    if (!board)
    {
        return;
    }

    if (board->getStatus() == BoardStatus::OFFLINE)
    {
        board->login();
    }
}

void MainWindow::boardwareInfoEvent(BoardPtr b, StringMap sp)
{
    QString bn = sp.getText("boardware");
    QString bv = sp.getText("version");

    _logger->trace("[{}][{}][{}]",
        b->getName().toStdString(), bn.toStdString(), bv.toStdString());
}
    
void MainWindow::onForumStructureChanged(BoardPtr b)
{
    if (qApp->activeModalWidget() == nullptr)
    {
        QString strTitle = QString("Forum '%1' Needs to be Updated").arg(b->getName());

        QString strMsg = QString(
            "The message board '%1' has changed. You must remove the board "
            "and add it back to Owl to see the changes.")
            .arg(b->getName());

        QMessageBox* msgBox = new QMessageBox(
            QMessageBox::Information,
            strTitle,
            strMsg,
            QMessageBox::Ok,
            this,
            Qt::Sheet);

        QString strDetails("The message board referenced above has been changed. Common changes include the message board administrator "
            "adding new forums, removing existing forums, or renaming a forum. These changes will not be reflected in Owl until you remove "
            "and add the board back to Owl. Note: Future versions of Owl will provide this functionality automatically.");

        msgBox->setDetailedText(strDetails);

        _logger->debug("Forum structure changed: {}", strMsg.toStdString());
        msgBox->open();
    }
}

void MainWindow::loginEvent(BoardPtr b, const StringMap& sp)
{
    auto doc = b->getBoardItemDocument();
    QString msg;

    _logger->debug("Board '{}' login result with user '{}' was {}",
        b->getName().toStdString(),
        b->getUsername().toStdString(),
        sp.getBool("success") ? "successful" : "unsuccessful");
    
    if (sp.getBool("success"))
    {
        if (_workerMap.contains(b->hash()))
        {
            QThreadEx* workerThread = _workerMap.value(b->hash());
            
            const QString threadName = QString("%1:%2:%3")
                .arg(b->getName()).arg(b->getServiceUrl()).arg(b->hash());
            workerThread->setObjectName(threadName);

            BoardUpdateWorker* pWorker = new BoardUpdateWorker(b);
            pWorker->moveToThread(workerThread);

            QObject::connect(pWorker, &BoardUpdateWorker::onForumStructureChanged,
                [this](BoardPtr board)
                {
                    QMetaObject::invokeMethod(this, "onForumStructureChanged", Q_ARG(owl::BoardPtr, board));
                });

            QObject::connect(workerThread, &QThread::started,
                [pWorker]()
                {
                    QMetaObject::invokeMethod(pWorker, "doWork");
                });

            QObject::connect(workerThread, &QThread::finished, 
                [workerThread, pWorker]()
                {
                    pWorker->setIsDone(true);
                    pWorker->deleteLater();
                    workerThread->deleteLater();
                });

            workerThread->start();
        }

        msg = QString(tr("User %1 signed on %2"))
            .arg(b->getUsername())
            .arg(b->getName());

        _logger->info(msg.toStdString());
    }
    else
    {
        msg = QString(tr("User %1 could not sign on to '%2'"))
            .arg(b->getUsername())
            .arg(b->getName());

        if (sp.has("error"))
        {
            msg += " because '" + sp.getText("error") + "'.";
        }
        else
        {
            msg += ".";
        }

        _logger->info(msg.toStdString());
    }

    QMainWindow::statusBar()->showMessage(msg, 5000);
}

// invoked when a request for posts has returned
void MainWindow::getPostsHandler(BoardPtr board, ThreadPtr thread)
{
    Q_UNUSED(board);

    QMutexLocker lock(&_updateMutex);

//    stopPostsLoading();

    if (thread->getPosts().size() > 0)
    {
//        postsWebView->showPosts(thread);
        updateSelectedThread(thread);
    }
    else
    {
        updateSelectedThread();
    }
}

void MainWindow::getThreadsHandler(BoardPtr /*b*/, ForumPtr forum)
{
    QMutexLocker lock(&_updateMutex);

//    stopThreadLoading();

//    if (forum->getThreads().size() > 0)
//    {
//        threadListWidget->setThreadList(forum->getThreads());
//    }

    updateSelectedForum(forum);
    contentView->doShowListOfThreads(forum);
}

// SLOT: handles the SIGNAL from a Board object. Called when the board responds 
// from a request to get a (flat) list of all forums with unread posts
// NOTE: duplicates should be removed before the list is handed to 
// this function
void MainWindow::getUnreadForumsEvent(BoardPtr board, ForumList list)
{
    _logger->debug("unread forum list retrieved for '{}' with {} forums", board->readableHash(), list.size());
    connectionView->update();
}

void MainWindow::onNewBoard()
{
    if (BOARDMANAGER->getBoardCount() >= MAX_BOARDS)
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(APP_TITLE);
        msgBox.setTextFormat(Qt::RichText);

        const QString strMsg = QString("You have already reached the maximum number of "
            "message boards you can add to Owl. Please remove a message board in "
            " order to add another");    

        msgBox.setText(strMsg);
        msgBox.exec(); 
    }
    else
    {
        _quickAddDlg = new QuickAddDlg(this);
        connect(_quickAddDlg, SIGNAL(newBoardAddedEvent(BoardPtr)), this, SLOT(onNewBoardAdded(BoardPtr)));

        _quickAddDlg->open();
    }
}

void MainWindow::onNewBoardAdded(BoardPtr board)
{
    _logger->info("new board '{}' added", board->readableHash());
    // TODO: initialize login here?
}

void MainWindow::connectBoard(BoardPtr board)
{
    connect(board.get(), SIGNAL(onLogin(BoardPtr, StringMap)),this, SLOT(loginEvent(BoardPtr, StringMap)));
    connect(board.get(), SIGNAL(onGetThreads(BoardPtr, ForumPtr)), this, SLOT(getThreadsHandler(BoardPtr, ForumPtr)));
    connect(board.get(), SIGNAL(onGetPosts(BoardPtr, ThreadPtr)), this, SLOT(getPostsHandler(BoardPtr, ThreadPtr)));
    connect(board.get(), SIGNAL(onGetUnreadForums(BoardPtr, ForumList)), this, SLOT(getUnreadForumsEvent(BoardPtr, ForumList)));
    connect(board.get(), SIGNAL(onMarkedForumRead(BoardPtr, ForumPtr)), this, SLOT(markForumReadHandler(BoardPtr, ForumPtr)));
    connect(board.get(), SIGNAL(onNewThread(BoardPtr, ThreadPtr)), this, SLOT(newThreadHandler(BoardPtr, ThreadPtr)));
    connect(board.get(), SIGNAL(onNewPost(BoardPtr, PostPtr)), this, SLOT(newPostHandler(BoardPtr, PostPtr)));

    QObject::connect(board.get(),
        &Board::onRequestError,
        [this](const Exception& ex)
        {
            this->_errorReportDlg = new ErrorReportDlg(ex, this);
            this->_errorReportDlg->show();

            // show the message in the status bar
            QMainWindow::statusBar()->showMessage(ex.message(), 5000);
        });
}

// called after the board's HTTP request to make a forum read returns   
void MainWindow::markForumReadHandler(BoardPtr b, ForumPtr f)
{
    _logger->info("Board '{}' had forum '{}' marked read", b->readableHash(), f->getName().toStdString());
    // TODO: reaload UI elements?
}

void MainWindow::newThreadHandler(BoardPtr board, ThreadPtr thread)
{
    _logger->trace("Thread successfully submitted to board '{}' with title '{}'",
        board->readableHash(), thread->getTitle().toStdString());

//    ForumPtr forum = this->getCurrentForum();

//    if (forum != nullptr)
//    {
//        startThreadLoading();
//        newThreadBtn->setEnabled(false);

//        b->requestThreadList(forum, ParserEnums::REQUEST_NOCACHE);
//    }

//    QMainWindow::statusBar()->showMessage("New thread sent", 5000);
}

void MainWindow::updateSelectedForum(ForumPtr f)
{
//    threadNavFrame->setEnabled(f != nullptr);
//    postsWebView->resetView();

    if (f != nullptr)
    {
//        newThreadBtn->setEnabled(f->getForumType() == Forum::FORUM);
//        threadPageNumEdit->setText(QString::number(f->getPageNumber()));
//        threadPageNumLbl->setText(QString::number(f->getPageCount()));
//        this->currentForumLbl->setText(f->getName());
    }
    else
    {
//        newThreadBtn->setEnabled(false);
//        this->currentForumLbl->setText(QString());
    }
}

// called from theadslist double-click handler and
// updates the UI
void MainWindow::updateSelectedThread(ThreadPtr t)
{
    // postNavFrame->setEnabled(t != nullptr);

    if (t != nullptr)
    {
//        newPostBtn->setEnabled(true);

//        postPageNumEdit->setText(QString::number(t->getPageNumber()));
//        postPageNumLbl->setText(QString::number(t->getPageCount()));

//        currentThreadLabel->setText(t->getTitle());

        if (t->hasUnread() && t->getPageNumber() == t->getPageCount())
        {
            t->setHasUnread(false);

            bool bForumHasUnread = false;
            ForumPtr parent = t->getParent()->upCast<ForumPtr>();
            for (auto th : parent->getThreads())
            {
                if (th->hasUnread())
                {
                    bForumHasUnread = true;
                }
            }

            if (!bForumHasUnread)
            {
//                servicesTree->markForumRead(parent);
            }

            update();
        }
    }
    else
    {
//		postsWebView->setThreadSelected(false);
//        newPostBtn->setEnabled(false);
//        currentThreadLabel->setText(QString());
    }
}

void MainWindow::onLinkActivated(const QString &urlStr)
{
    // TODO: test, this function changed a lot going from Qt4 -> Qt5
    QUrl url(urlStr);
    QUrlQuery str(urlStr);

    if (url.scheme() == "owl")
    {
        // paths come through as '/boardconfig' so strip off the '/'
        QString path(url.path().remove(0,1));
        
        if (_linkMessageMap.contains(path))
        {
            _linkMessageMap.value(path)(str.queryItems());
            
        }
        else
        {
            _logger->warn("unknown url.path '{}' in url '{}'", url.path().toStdString(), urlStr.toStdString());
        }
    }
}

void MainWindow::toggleOldControls(bool doshow)
{
//    currentForumFrame->setVisible(doshow);
//    threadNavFrame->setVisible(doshow);
//    threadListWidget->setVisible(doshow);
//    line->setVisible(doshow);
//    line_3->setVisible(doshow);

//    currentThreadFrame->setVisible(doshow);
//    postsWebView->setVisible(doshow);
//    postNavFrame->setVisible(doshow);
//    line_2->setVisible(doshow);
//    line_4->setVisible(doshow);
}

void MainWindow::createDebugMenu()
{
    QMenu* debugMenu = this->menuBar()->addMenu("&Debug");

    {
        QAction* action = debugMenu->addAction("&Show Splash Screen");

        QObject::connect(action, &QAction::triggered,
            [this]()
            {
                _splash->show();
            });
    }

    {
        QAction* action = debugMenu->addAction("&Show INI Settings Folder");
        QObject::connect(action, &QAction::triggered, []()
        {
            const QString writePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
            const QString iniFile = QDir(writePath).absoluteFilePath("owl.ini");
            const QFileInfo fileInfo(iniFile);
            owl::openFolder(fileInfo.absolutePath());
        });
    }

    {
        QAction* action = debugMenu->addAction("&Show Show Parsers Folder");
        QObject::connect(action, &QAction::triggered, []()
        {
            const auto parsersFolder = SettingsObject().read("parsers.path").toString();
            owl::openFolder(parsersFolder);
        });
    }

    debugMenu->addSeparator();

    // Display a dialog used to write posts, for quick testing
    {
        QAction* action = debugMenu->addAction("&Show PostTextEditor dialog");
        QObject::connect(action, &QAction::triggered,
            []()
            {
                ParserBasePtr parser = PARSERMGR->createParser("tapatalk4x", "http://www.amb.la");
                BoardPtr board = std::make_shared<Board>();
                board->setParser(parser);

                ForumPtr forum = std::make_shared<Forum>("TESTFORUM");
                forum->setBoard(board);

                NewThreadDlg dlg(forum);
                dlg.exec();
            });
    }

    debugMenu->addSeparator();

    {
        QAction* action = debugMenu->addAction("&Toggle Old View");
        action->setShortcut(QKeySequence("Ctrl+Shift+1"));
        QObject::connect(action, &QAction::triggered,
            [this]()
            {
//                bool toggle = !currentForumFrame->isVisible();
//                toggleOldControls(toggle);
            });
    }
}

void MainWindow::createMenus()
{
    // File menu
    {
        QMenu* menu = this->menuBar()->addMenu(tr("&File"));
        
        // File/New Board menu item
        QAction* newBoard = menu->addAction(tr("New Board"));
        newBoard->setToolTip("Add a new board");
        newBoard->setStatusTip("Add a new board");
        newBoard->setShortcut(QKeySequence("Ctrl+N"));
        QObject::connect(newBoard, SIGNAL(triggered()), this, SLOT(onNewBoard()));
        
        menu->addSeparator();
        
        // File/Preferences menu item
        {
            QAction* settings = menu->addAction(tr("&Preferences"));
            settings->setToolTip(tr("Preferences"));
            settings->setStatusTip(tr("Edit Owl preferences"));
            settings->setMenuRole(QAction::PreferencesRole);
            settings->setShortcut(QKeySequence::Preferences);
            QObject::connect(settings, &QAction::triggered, [this]()
            {
                openPreferences();
            });
        }

#ifdef Q_OS_WIN
        menu->addSeparator();
#endif
        
        {
            QAction* quit = menu->addAction("E&xit");
            quit->setMenuRole(QAction::QuitRole);
            quit->setShortcut(QKeySequence::Quit);
            
            // MainWindow::closeEvent() wasn't getting triggered until
            // I changed the connection of the "Exit" per this thread here
            // http://stackoverflow.com/questions/23274983/qt-mainwindow-closeevent-mac-cmdq
            QObject::connect(quit, &QAction::triggered,[this]() { close(); });
        }
    }
    
    // Edit menu
    {
        QMenu* menu = menuBar()->addMenu("&Edit");
        
        auto undo = menu->addAction("Undo");
        undo->setShortcut(QKeySequence("Ctrl+Z"));
        QObject::connect(undo, &QAction::triggered,
            [this]()
            {
                auto w = QApplication::activeWindow()->focusWidget();

                // try to cast to a QLineEdit
                QLineEdit* le = qobject_cast<QLineEdit*>(w);
                if (le)
                {
                    _logger->trace("Undo on Item {}", le->objectName().toStdString());
                    le->undo();
                }
            });
        
        auto redo = menu->addAction("Redo");
        redo->setShortcut(QKeySequence("Shift+Ctrl+Z"));
        QObject::connect(redo, &QAction::triggered,
            [this]()
            {
                auto w = QApplication::activeWindow()->focusWidget();

                // try to cast to a QLineEdit
                QLineEdit* le = qobject_cast<QLineEdit*>(w);
                if (le)
                {
                    _logger->trace("Redo on Item {}", le->objectName().toStdString());
                    le->redo();
                }
         });
        
        menu->addSeparator();
        
        auto cut = menu->addAction("Cut");
        cut->setShortcut(QKeySequence("Ctrl+X"));
        
        
        auto copy = menu->addAction("Copy");
        copy->setShortcut(QKeySequence("Ctrl+C"));
        
        auto paste = menu->addAction("Paste");
        paste->setShortcut(QKeySequence("Ctrl+V"));
        
        menu->addSeparator();
        
        auto selectAll = menu->addAction("Select All");
        selectAll->setShortcut(QKeySequence("Ctrl+A"));
        
        auto deselectAll = menu->addAction("Deselect All");
        deselectAll->setShortcut(QKeySequence("Shift+Ctrl+A"));
        
        QObject::connect(menu, &QMenu::aboutToShow, [=]()
         {
             auto activeWindow = QApplication::activeWindow();
             auto widget = activeWindow->focusWidget();

             if (widget == connectionView)
             {
                 undo->setEnabled(false);
                 redo->setEnabled(false);
                 cut->setEnabled(false);
                 copy->setEnabled(false);
                 paste->setEnabled(false);
                 selectAll->setEnabled(false);
                 deselectAll->setEnabled(false);
             }
     });
    }

    // View menu
    {
        QMenu* viewMenu = this->menuBar()->addMenu("&View");
        
        {
            QAction* action = viewMenu->addAction("Enter Full Screen");
            action->setShortcut(QKeySequence::FullScreen);
            QObject::connect(action, &QAction::triggered, [this,action]()
            {
                if (this->isFullScreen())
                {
                    this->showNormal();
                    action->setText(tr("Enter Full Screen"));
                }
                else
                {
                    this->showFullScreen();
                    action->setText(tr("Exit Full Screen"));
                }
            });
        }
        
        viewMenu->addSeparator();
        
        {
            auto boardPaneMenu = viewMenu->addAction("Hide Boards Pane");
            QObject::connect(boardPaneMenu, &QAction::triggered, [this,boardPaneMenu]()
            {
                connectionView->setVisible(!connectionView->isVisible());
                
                if (connectionView->isVisible())
                {
                    boardPaneMenu->setText(tr("Hide Boards Pane"));
                }
                else
                {
                    boardPaneMenu->setText(tr("Show Boards Pane"));
                }
            });
        }
        
        // View Menu -> Posts Pane
        {
            QMenu* postsPaneMenu = viewMenu->addMenu("Posts Pane");

            {
                // View Menu -> Posts Pane -> Right
                _actions.postPaneRight = postsPaneMenu->addAction("Right");
                _actions.postPaneRight->setCheckable(true);
                QObject::connect(_actions.postPaneRight, &QAction::triggered, [this]()
                {
                    // if (!postViewDockWidget->isVisible())
                    // {
                    //     // postViewDockWidget->setTitleBarWidget(new QWidget(this));
                    //     postViewDockWidget->setVisible(true);
                    // }

                    // addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, postViewDockWidget);
                    this->_actions.postPaneRight->setChecked(true);
                    this->_actions.postPaneBelow->setChecked(false);
                    this->_actions.postPaneHidden->setChecked(false);

                    _postsPanePosition = PANERIGHT;
                });
                
                // View Menu -> Posts Pane -> Below
                _actions.postPaneBelow = postsPaneMenu->addAction("Below");
                _actions.postPaneBelow->setCheckable(true);
                QObject::connect(_actions.postPaneBelow, &QAction::triggered, [this]()
                {
                    // if (!postViewDockWidget->isVisible())
                    // {
                    //     // postViewDockWidget->setTitleBarWidget(new QWidget(this));
                    //     postViewDockWidget->setVisible(true);
                    // }

                    // addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, postViewDockWidget);
                    this->_actions.postPaneRight->setChecked(false);
                    this->_actions.postPaneBelow->setChecked(true);
                    this->_actions.postPaneHidden->setChecked(false);

                    _postsPanePosition = PANEBOTTOM;
                });
                
                // View Menu -> Posts Pane -> Hidden
                _actions.postPaneHidden = postsPaneMenu->addAction("Hidden");
                _actions.postPaneHidden->setCheckable(true);
                QObject::connect(_actions.postPaneHidden, &QAction::triggered, [this]()
                {
                    // postViewDockWidget->setVisible(false);
                    _actions.postPaneRight->setChecked(false);
                    _actions.postPaneBelow->setChecked(false);
                    _actions.postPaneHidden->setChecked(true);
                    
                    _postsPanePosition = PANEHIDDEN;
                });
            }
        }

        viewMenu->addSeparator();

        {
            QAction* action = viewMenu->addAction("Show Board Toolbar");
            QObject::connect(action, &QAction::triggered,
                [this]()
                {
                    boardToolbar->setVisible(!boardToolbar->isVisible());
                });

            _actions.showBoardbar = action;
        }

        viewMenu->addSeparator();

        if (owl::isWindowsHost())
        {
            QAction* action = viewMenu->addAction("Hide menu");
            QObject::connect(action, &QAction::triggered, [this]()
            {
                this->showMenuBar(false);
            });
        }

        // Status Bar menu settings and initialization for startup
        {
            QAction* action = viewMenu->addAction("Show Status Bar");
            action->setToolTip("Toggle the display of the status bar");
            QObject::connect(action, &QAction::triggered, [this,action]()
            {
                if (QMainWindow::statusBar()->isVisible())
                {
                    QMainWindow::statusBar()->hide();
                    action->setText(tr("Show Status Bar"));
                }
                else
                {
                    QMainWindow::statusBar()->show();
                    action->setText(tr("Hide Status Bar"));                    
                }
                
                _statusBarVisibile = !_statusBarVisibile;
                action->setChecked(QMainWindow::statusBar()->isVisible());
            });
            
            if (_statusBarVisibile)
            {
                QMainWindow::statusBar()->show();
                action->setText(tr("Hide Status Bar"));
            }
            else
            {
                QMainWindow::statusBar()->hide();
                action->setText(tr("Show Status Bar"));
            }
            
            _actions.showStatusBar = action;
        }
    }
    
    // Help Menu
    {
        QMenu* helpMenu = this->menuBar()->addMenu("&Help");

        {
            QAction* action = helpMenu->addAction(tr("Documentation"));
            action->setShortcut(QKeySequence::HelpContents);
            QObject::connect(action, &QAction::triggered, [this]()
            {
                QUrl url("http://wiki.owlclient.com");
                _logger->trace("Launching browser: {}", url.toString().toStdString());
                QDesktopServices::openUrl(url);
            });
        }

        {
            QAction* action = helpMenu->addAction(tr("Release Notes"));
            QObject::connect(action, &QAction::triggered, [this]()
            {
                auto urlStr = QString("http://wiki.owlclient.com/index.php?title=Release_Notes_%1").arg(OWL_VERSION);
                QUrl url(urlStr);
                _logger->trace("Launching browser: {}", url.toString().toStdString());
                QDesktopServices::openUrl(url);
            });
        }

        helpMenu->addSeparator();

        {
            QAction* action = helpMenu->addAction(tr("Donate"));
            QObject::connect(action, &QAction::triggered, [this]()
            {
                auto urlStr = QString("https://www.paypal.me/zethon");
                QUrl url(urlStr);
                _logger->trace("Launching browser: {}", url.toString().toStdString());
                QDesktopServices::openUrl(url);
            });
        }

        {
            QAction* action = helpMenu->addAction(tr("&Owl on Twitter"));
            QObject::connect(action, &QAction::triggered, [this]()
            {
                QUrl url("http://www.twitter.com/OwlClient");
                _logger->trace("Launching browser: {}", url.toString().toStdString());
                QDesktopServices::openUrl(url);
            });
        }

        {
            QAction* action = helpMenu->addAction(tr("&Report an issue"));
            QObject::connect(action, &QAction::triggered, [this]()
            {
                QUrl url("http://bugs.owlclient.com");
                _logger->trace("Launching browser: {}", url.toString().toStdString());
                QDesktopServices::openUrl(url);

            });
        }

        helpMenu->addSeparator();

#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
        {
            QAction* action = helpMenu->addAction(tr("Show Log Folder"));
            QObject::connect(action, &QAction::triggered, [&]()
            {
                SettingsObject settings;
                if (settings.read("logs.file.enabled").toBool())
                {
                    QMessageBox::warning(this, APP_TITLE,
                        tr("Logging is not enabled.\n\nTurn on logging in Preferences->Advanced to configure."), QMessageBox::Ok);
                }
                else
                {
                    QFileInfo fileInfo(settings.read("logs.file.path").toString());
                    if (fileInfo.isReadable())
                    {
                        owl::openFolder(fileInfo.absolutePath());
                    }
                    else
                    {
                        QMessageBox::warning(this, APP_TITLE,
                            tr("Invalid log file location.\n\nCheck Preferences->Advanced to configure."), QMessageBox::Ok);
                    }
                }
            });
        }

        helpMenu->addSeparator();
#endif

        {
            QAction* action = helpMenu->addAction("&About...");
            action->setMenuRole(QAction::AboutRole);

            QObject::connect(action, &QAction::triggered, [this]()
            {
                AboutDlg about(this);
                about.exec();
            });
        }
    }

#ifndef RELEASE
    // Debug Menu
    createDebugMenu();
#endif

}

void MainWindow::createStatusBar()
{
    QToolButton* bvbtn = new QToolButton(this);
    bvbtn->setIcon(QIcon(":/icons/boardview-expand-button.png"));
    bvbtn->setAutoRaise(true);
    bvbtn->setStyleSheet("QToolButton { background-color: transparent; } QToolButton:hover { background-color: #D0D0D0; border-radius: 2px; }");

    QObject::connect(bvbtn, &QToolButton::clicked,
        [this](bool)
        {
            // TODO: need to toggle the visibility of the `BoardIconView`
            _logger->trace("Toggling visibility of boards view");
        });

    QMainWindow::statusBar()->addWidget(bvbtn);
    QMainWindow::statusBar()->setMaximumHeight(20);
}

void MainWindow::navigateToPostListPage(ThreadPtr thread, int iPageNumber)
{
    auto board = thread->getBoard().lock();

    if (board && iPageNumber != thread->getPageNumber())
    {
        if (iPageNumber > thread->getPageCount())
        {
            iPageNumber = thread->getPageCount();
        }
        else if (iPageNumber < 1)
        {
            iPageNumber = 1;
        }
        
//        startPostsLoading();
        thread->setPageNumber(iPageNumber);
        board->requestPostList(thread, ParserEnums::REQUEST_DEFAULT, true);
//        postPageNumEdit->setText(QString::number(iPageNumber));
    }
}
    
void MainWindow::navigateToThreadListPage(ForumPtr forum, int iPageNumber)
{
    auto board = forum->getBoard().lock();

    if (board && iPageNumber != forum->getPageNumber())
    {
        if (iPageNumber > forum->getPageCount())
        {
            iPageNumber = forum->getPageCount();
        }
        else if (iPageNumber < 1)
        {
            iPageNumber = 1;
        }

//        startThreadLoading();
//        threadPageNumEdit->setText(QString::number(iPageNumber));
        forum->setPageNumber(iPageNumber);
        board->requestThreadList(forum);
    }
}

//void MainWindow::newPostBtnClicked()
//{
//    auto threadPtr = this->threadListWidget->getCurrentThread().lock();
//    if (threadPtr)
//    {
//        NewThreadDlg* dlg = new NewThreadDlg(threadPtr, this);
//        dlg->setModal(false);
//        dlg->show();
//    }
//}

// event sent from Board object notifying the UI that a new post
// has been successfully posted
void MainWindow::newPostHandler(BoardPtr b, PostPtr p)
{
    if (p)
    {
//        ThreadPtr thread = this->threadListWidget->getCurrentThread().lock();

//        if (thread && thread == p->getParent())
//        {
//            this->startPostsLoading();
////            this->newPostBtn->setEnabled(false);
//            b->requestPostList(thread, ParserEnums::REQUEST_NOCACHE);
//        }
//        else
//        {
//            Q_ASSERT(p->getParent() != nullptr);
//            QString msg = QString("New post successfully sent in thread '%1'").arg(p->getParent()->getTitle());
//            QMainWindow::statusBar()->showMessage(msg, 5000);
//        }
    }
    else
    {
        _logger->warn("Null new post returned for board {} ({})",
            b->getName().toStdString(), b->getDBId());

        QMainWindow::statusBar()->showMessage("New post saved", 5000);
    }
}

void MainWindow::createLinkMessages()
{
    _linkMessageMap.insert("boardconfig", std::bind(&MainWindow::onNewBoard, this));
}

void MainWindow::createBoardPanel()
{
    // NEW
#ifdef Q_OS_MACX
    connectionView->setAttribute(Qt::WA_MacShowFocusRect, 0);
#endif

    QObject::connect(connectionView, &BoardIconView::onBoardClicked,
        [this](owl::BoardWeakPtr bwp)
        {
            contentView->doShowLoading(bwp);
            threadListWidget2->doBoardClicked(bwp);
        });

    QObject::connect(connectionView, &BoardIconView::onEditBoard,
        [this](owl::BoardWeakPtr boardWeakPtr)
        {
            auto boardPtr = boardWeakPtr.lock();
            if (!boardPtr) OWL_THROW_EXCEPTION(owl::Exception("Invalid board"));

            EditBoardDlg* dlg = new EditBoardDlg(boardPtr, this);

            QObject::connect(dlg, &EditBoardDlg::boardSavedEvent,
                [this](const BoardPtr b, const StringMap&)
            {
                _logger->trace("onBoardInfoSaved({}:{})",
                    b->getDBId(), b->getName().toStdString());

                //auto doc = b->getBoardItemDocument();
                //doc->setOrAddVar("%BOARDNAME%", b->getName());
                //doc->setOrAddVar("%BOARDUSERNAME%", b->getUsername());
                //doc->reloadHtml();

                // search the toolbar (top of the client) and update the text
                for (QAction* a : boardToolbar->actions())
                {
                    if (b == a->data().value<BoardWeakPtr>().lock())
                    {
                        a->setText(b->getName());
                        a->setIconText(owl::getAbbreviatedName(b->getName()));
                        QString toolTip = QString("%1@%2").arg(b->getUsername()).arg(b->getName());
                        a->setToolTip(toolTip);
                        break;
                    }
                }
            });

            QObject::connect(dlg, &QDialog::finished, [dlg](int) { dlg->deleteLater(); });
            dlg->open();
        });

    QObject::connect(connectionView, &BoardIconView::onAddNewBoard,
        [this]()
        {
            QuickAddDlg* addDlg = new QuickAddDlg(this);
            connect(addDlg, SIGNAL(newBoardAddedEvent(BoardPtr)), this, SLOT(onNewBoardAdded(BoardPtr)));

            connect(addDlg, &QuickAddDlg::newBoardAddedEvent,
                [this](BoardPtr board)
                {
                    if (this->initBoard(board))
                    {
//                        board->login();
                    }
                });

            QObject::connect(addDlg, &QDialog::finished, [addDlg](int) { addDlg->deleteLater(); });
            addDlg->open();
        });

    QObject::connect(connectionView, &BoardIconView::onDeleteBoard,
        [this](owl::BoardWeakPtr bwp)
        {
            BoardPtr board = bwp.lock();
            if (board)
            {
                QThreadEx* thread = _workerMap.value(board->hash());
                Q_ASSERT(thread);
                thread->exit();

                // NOTE: More of Qt's lifetime management, this will, in fact, delete
                // the `BoardUpdateWorker` and `QThreadEx` objects that are in the
                // `_workerMap`
                _workerMap.remove(board->hash());

                // delete the board from the toolbar
                auto actionList = boardToolbar->actions();
                auto actionItem = std::find_if(actionList.begin(), actionList.end(),
                    [board](QAction* action)
                    {
                        auto other = action->data().value<BoardWeakPtr>().lock();
                        return board && other && board->hash() == other->hash();
                    });

                if (actionItem != actionList.end())
                {
                    boardToolbar->removeAction(*actionItem);

                }

                // remove from the database
                BOARDMANAGER->deleteBoard(board);

                // clear any resources
                board.reset();
            }
        });

    QObject::connect(connectionView, &BoardIconView::onConnectBoard,
        [this](owl::BoardWeakPtr bwp)
        {
            BoardPtr board = bwp.lock();
            if (board)
            {
                _logger->info("Connecting board {}", board->getName().toStdString());
                board->login();
            }
        });
}
    
void MainWindow::createThreadPanel()
{
    QObject::connect(threadListWidget2, &ForumView::onForumClicked,
        [this](owl::ForumPtr forum)
        {
            BoardPtr board = forum->getBoard().lock();
            if (board && board->getStatus() == BoardStatus::ONLINE)
            {
                contentView->doShowLoading(board);
                board->requestThreadList(forum);
                board->setLastForumId(forum->getId().toInt());
            }
        });

    QObject::connect(threadListWidget2, &ForumView::onForumListLoaded,
        [this]() { contentView->doShowLogo(); });
}

void MainWindow::createPostPanel()
{
    // // @TODO: Is this still needed?
    // QObject::connect(postViewDockWidget, &QDockWidget::visibilityChanged, 
    //     [this](bool bVisible)
    // {
    //     this->_actions.postPaneHidden->setChecked(!bVisible);
    // });
}

// Invoked from the board toolbar when user clicks 'Open in Browser'
void MainWindow::onOpenBrowserToolbar()
{
    QAction* caller = qobject_cast<QAction*>(sender());

    if (caller != nullptr)
    {
        QString url;

        if (caller->data().canConvert<BoardWeakPtr>())
        {
            BoardPtr b = caller->data().value<BoardWeakPtr>().lock();
            url = b->getUrl();
        }
        else if (caller->data().canConvert<ForumPtr>())
        {
            ForumPtr f = caller->data().value<ForumPtr>();
            auto board = f->getBoard().lock();
            if (board)
            {
                url = board->getParser()->getItemUrl(f);
            }
        }

        if (!url.isEmpty())
        {
            QDesktopServices::openUrl(url);
        }	
    }
}

void MainWindow::onCopyUrl()
{
    QAction* caller = qobject_cast<QAction*>(sender());

    if (caller != nullptr)
    {
        QString url;
        BoardItemPtr boardItem;

        if (caller->data().canConvert<BoardWeakPtr>())
        {
            BoardPtr b = caller->data().value<BoardWeakPtr>().lock();
            url = b->getUrl();
        }
        else if (caller->data().canConvert<ForumPtr>())
        {
            ForumPtr f = caller->data().value<ForumPtr>();

            auto board = f->getBoard().lock();
            if (board)
            {
                url = board->getParser()->getItemUrl(f);
            }
        }

        if (!url.isEmpty())
        {
            qApp->clipboard()->setText(url);
        }	
    }
}

void MainWindow::onBoardDelete()
{
    QAction* caller = qobject_cast<QAction*>(sender());

    if (caller != nullptr && caller->data().canConvert<BoardWeakPtr>())
    {
        BoardPtr b = caller->data().value<BoardWeakPtr>().lock();
        QString strMsg = QString(tr("Are you sure you want to delete the board \"%1\"?\n\n"
            "This will permanently delete the set up of the account.")).arg(b->getName());

        QMessageBox* messageBox = new QMessageBox(
            QMessageBox::Question,
            tr("Delete Message Board"),
            strMsg,
            QMessageBox::Yes | QMessageBox::No,
            this, 
            Qt::Drawer);

        messageBox->setWindowModality(Qt::WindowModal);

        if (messageBox->exec() == QMessageBox::Yes)
        {
//			onBoardDelete(b);
            QMetaObject::invokeMethod(this, "onBoardDelete", Q_ARG(BoardPtr, b));
        }
    }
}
    
void MainWindow::onBoardDelete(BoardPtr b)
{
    // stop any pending requests
    QThread* thread = _workerMap.value(b->hash());
    thread->exit();

    _workerMap.remove(b->hash());

    // search the toolbar (top of the client) and
    // remove the board icon
    for (QAction* a : boardToolbar->actions())
    {
        if (b == a->data().value<BoardWeakPtr>().lock())
        {
            boardToolbar->removeAction(a);
            boardToolbar->resize(boardToolbar->sizeHint());
            break;
        }
    }

    // remove the board from the database
    BOARDMANAGER->deleteBoard(b);

    if (BOARDMANAGER->getBoardCount() == 0)
    {
        update();
        updateSelectedForum(ForumPtr());
        updateSelectedThread(ThreadPtr());

        setWindowTitle(QStringLiteral(APP_NAME));
    }

    b.reset();
}

void MainWindow::showMenuBar(bool visible) const
{
    menuBar()->setVisible(visible);
#if defined(Q_OS_WIN)
    if (visible)
    {
        ::setShowMenuText(winId(), "Hide Menu");
    }
    else
    {
        ::setShowMenuText(winId(), "Show Menu");
    }
#endif
}

void MainWindow::readWindowSettings()
{
    const QString writePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    const QString iniFile = QDir(writePath).absoluteFilePath("owl.ini");

    if (QFile(iniFile).exists())
    {
        _logger->debug("Loading MainWindow settings from '{}'", iniFile.toStdString());
        QSettings settings(iniFile, QSettings::IniFormat);

        settings.beginGroup("MainWindow");

        const auto geometry = settings.value("geometry").value<QByteArray>();
        this->restoreGeometry(geometry);

        const auto state = settings.value("state").value<QByteArray>();
        this->restoreState(state);

        const auto statusBarGeometry = settings.value("statusBarGeometry").value<QByteArray>();
        QMainWindow::statusBar()->restoreGeometry(statusBarGeometry);

        _statusBarVisibile = settings.value("statusBarVisible").toBool();
        _postsPanePosition = settings.value("postsPanePosition").toUInt();

        menuBar()->setVisible(settings.value("showMenuBar").toBool());
    }
    else
    {
        boardToolbar->setVisible(false);
        _logger->info("No settings file found at '{}', using defaults", iniFile.toStdString());
    }
}

void MainWindow::writeWindowSettings()
{
    const QString writePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    const QString iniFile = QDir(writePath).absoluteFilePath("owl.ini");

    QSettings settings(iniFile, QSettings::IniFormat);
    settings.beginGroup("MainWindow");

    settings.setValue("geometry",saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("statusBarGeometry", QMainWindow::statusBar()->saveGeometry());
    settings.setValue("statusBarVisible", _statusBarVisibile);
    settings.setValue("postsPanePosition", _postsPanePosition);
    settings.setValue("showMenuBar", menuBar()->isVisible());
}
                                                             
void MainWindow::onDisplayOrderChanged(BoardPtr b, int iDirection)
{	
    QMutexLocker locker(&_updateMutex);

    _logger->debug("Display order changed, moving board '{}' in direction {}", b->readableHash(), iDirection);

    // search through the toolbar looking for the board
    auto actionIdx = -1;
    QAction* tbAction = nullptr;
    for (auto a : boardToolbar->actions())
    {
        actionIdx++;

        if (a->data().canConvert<BoardWeakPtr>() && a->data().value<BoardWeakPtr>().lock()->getDBId() == b->getDBId())
        {
            tbAction = a;
            break;
        }
    }

    // if we found the board in the boardToolbar (and we should have) then and only
    // then do we move it
    if (tbAction)
    {
//        if (iDirection > 0)
//        {
//            auto beforeIdx = actionIdx - +iDirection + 1;
//            auto numActions = boardToolbar->actions().size();
            
//            if (beforeIdx >= numActions)
//            {
//                boardToolbar->addAction(tbAction);
//            }
//            else
//            {
//                boardToolbar->removeAction(tbAction);
                
//                // iDirection + 1 since we have to insert 'before'
//                auto otherIdx = row + iDirection + 1;
//                if (otherIdx >= boardToolbar->actions().size())
//                {
//                    // board icon is being moved to last position
//                    boardToolbar->addAction(tbAction);
//                }
//                else
//                {
//                    QAction* otherAction = boardToolbar->actions().at(otherIdx);
//                    boardToolbar->insertAction(otherAction, tbAction);
//                }
//            }
//        }
//        else if (iDirection < 0)
//        {
//            auto otherIdx = actionIdx - 1;
//            QAction* otherAction = boardToolbar->actions().at(otherIdx);

//            if (otherAction)
//            {
//                boardToolbar->removeAction(tbAction);
//                boardToolbar->insertAction(otherAction, tbAction);
//            }
//        }
    }
    else
    {
        _logger->warn("Board '{}' not found in boardToolbar", b->readableHash());
    }
}

void BoardMenu::onAboutToShow()
{
    // make things a little faster
    auto board = _board.lock();

    if (board && board->getStatus() != _lastStatus)
    {
        createMenu();
    }
}

void BoardMenu::createMenu()
{
    MainWindow* parent = qobject_cast<MainWindow*>(this->parent());

    clear();

    auto board = _board.lock();
    if (!board)
    {
        OWL_THROW_EXCEPTION(Exception("Board object is null"));
    }

    if (board->getStatus() == BoardStatus::ONLINE)
    {
        QAction* refresh = addAction(QIcon(":/icons/refresh.png"), tr("Refresh"));
        refresh->setToolTip(tr("Refresh"));
#ifdef Q_OS_MACX
        refresh->setIconVisibleInMenu(false);
#endif
        QObject::connect(refresh, &QAction::triggered, [this]()
        {
            auto board = _board.lock();
            if (board)
            {
                board->updateUnread();
            }
        });

//		auto signOut = addAction("Disconnect");
//		signOut->setToolTip(tr("Disconnect"));
//		QObject::connect(signOut, &QAction::triggered, [this]()
//		{
//			// TODO: signoff
//		});
    }
    else
    {
        QAction* action = addAction(tr("Connect"));
        action->setToolTip(tr("Connect"));
        QObject::connect(action, &QAction::triggered, [this]()
        {
            auto board = _board.lock();

            if (board)
            {
                board->login();
            }
        });
    }

    addSeparator();

    {
        QAction* action = addAction(tr("Copy Board Address"));
        action->setToolTip(tr("Copy Board Address"));
        connect(action, &QAction::triggered, [=]()
        {
            auto board = _board.lock();

            if (board)
            {
                auto url = board->getUrl();
                qApp->clipboard()->setText(url);
            }
        });
    }

    {
        QAction* action = addAction(QIcon(":/icons/link.png"), tr("Open in Browser"));
        action->setToolTip(tr("Open in Browser"));
#ifdef Q_OS_MACX
        action->setIconVisibleInMenu(false);
#endif

        connect(action, &QAction::triggered, [=]()
        {
            auto board = _board.lock();

            if (board)
            {
                auto url = board->getUrl();
                QDesktopServices::openUrl(url);
            }
        });
    }

    addSeparator();

    if (board->getStatus() == BoardStatus::ONLINE)
    {
        QAction* action = addAction(QIcon(":/icons/markforumread.png"), tr("Mark All Forums Read"));
        action->setToolTip(tr("Mark All Forums Read"));
#ifdef Q_OS_MACX
        action->setIconVisibleInMenu(false);
#endif

        connect(action, &QAction::triggered, [=]()
        {
            auto board = _board.lock();
            if (board)
            {
                board->markForumRead(board->getRoot());
            }
        });
    }

    {
        QAction* action = addAction(QIcon(":/icons/settings.png"), tr("Settings"));
#ifdef Q_OS_MACX
        action->setIconVisibleInMenu(false);
#endif

        connect(action, &QAction::triggered, [=]()
        {
            try
            {
                auto board = _board.lock();
                if (!board)
                {
                    OWL_THROW_EXCEPTION(WebException("Invalid board handle"));
                }

                EditBoardDlg* pDlg = new EditBoardDlg(board, parent);
                QObject::connect(pDlg, SIGNAL(boardSavedEvent(const BoardPtr, const StringMap&)), this, SIGNAL(boardInfoSaved(const BoardPtr, const StringMap&)));
                pDlg->open();
            }
            catch (const owl::Exception& owlex)
            {
                std::stringstream str;
                str << "There was an error opening the settings dialog: " << owlex.message().toStdString();
                QMessageBox::warning(nullptr, APP_TITLE, QString::fromStdString(str.str()));
            }
            catch (const std::exception& ex)
            {
                std::stringstream str;
                str << "There was an error opening the settings dialog: " << ex.what();
                QMessageBox::warning(nullptr, APP_TITLE, QString::fromStdString(str.str()));
            }
        });
    }

    addSeparator();

    {
        QAction* action = addAction(QIcon(":/icons/delete.png"), tr("Delete"));
        action->setData(QVariant::fromValue(_board));
#ifdef Q_OS_MACX
        action->setIconVisibleInMenu(false);
#endif

        connect(action, SIGNAL(triggered()), parent, SLOT(onBoardDelete()));
    }
}

void ImageOverlay::newParent()
{
    if (!parent())
    {
        return;
    }

    parent()->installEventFilter(this);
    raise();
}

} // namespace owl

