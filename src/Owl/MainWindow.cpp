// Owl - www.owlclient.com
// Copyright (c) 2012-2023, Adalid Claure <aclaure@gmail.com>

#include <QDomElement>
#include <QQuickItem>
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
#include "PostTextEditor.h"
#include "NewThreadDlg.h"

#ifdef Q_OS_WIN
#include "windows.h"
#elif defined(Q_OS_MACOS)
#include <QtMac>
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
      _logger(owl::initializeLogger("MainWindow"))
{
    setupUi(this);

    initializeTitleBar(this);

    // TODO: move this to the OwlApplication class
    readWindowSettings();
    
    // initialize the dictionaries
    SPELLCHECKER->init();

    QMainWindow::statusBar()->hide();
    QTimer::singleShot(0, this, SLOT(onLoaded()));
}

void MainWindow::onLoaded()
{
    loadBoards();
    createBoardPanel();
    createThreadPanel();
    updateSelectedThread();
    createMenus();
}

void MainWindow::loadBoards()
{
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
}

bool MainWindow::initBoard(const BoardPtr& b)
{
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
    PreferencesDlg* dlg = new PreferencesDlg(this);

    QObject::connect(dlg, &PreferencesDlg::onBoardEdit, this,
        [this](const BoardPtr board, BoardEditAction action)
        {
            Q_ASSERT(board != nullptr);
            if (action == BoardEditAction::Delete)
            {
               onBoardDelete(board);
            }
            else if (action == BoardEditAction::MoveUp)
            {
                OWL_THROW_EXCEPTION(owl::NotImplementedException());
            }
            else if (action == BoardEditAction::MoveDown)
            {
                OWL_THROW_EXCEPTION(owl::NotImplementedException());
            }
        });

    QObject::connect(dlg, &QDialog::finished, [dlg](int) { dlg->deleteLater(); });
    // dlg->setWindowFlags(dlg->windowFlags() | Qt::Popup);
    dlg->open();
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
                .arg(b->getName(), b->getServiceUrl()).arg(b->hash());
            workerThread->setObjectName(threadName);

            BoardUpdateWorker* pWorker = new BoardUpdateWorker(b);
            pWorker->moveToThread(workerThread);

            QObject::connect(pWorker, &BoardUpdateWorker::onForumStructureChanged, this,
                [this](BoardPtr board)
                {
                    QMetaObject::invokeMethod(this, "onForumStructureChanged", Q_ARG(owl::BoardPtr, board));
                });

            QObject::connect(workerThread, &QThread::started, this,
                [pWorker]()
                {
                    QMetaObject::invokeMethod(pWorker, "doWork");
                });

            QObject::connect(workerThread, &QThread::finished, this,
                [workerThread, pWorker]()
                {
                    pWorker->setIsDone(true);
                    pWorker->deleteLater();
                    workerThread->deleteLater();
                });

            workerThread->start();
        }

        msg = QString(tr("User %1 signed on %2"))
            .arg(b->getUsername(), b->getName());

        _logger->info(msg.toStdString());
    }
    else
    {
        msg = QString(tr("User %1 could not sign on to '%2'"))
            .arg(b->getUsername(), b->getName());

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

    if (thread->getPosts().size() > 0)
    {
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
    forumContentView->doShowListOfThreads(forum);
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
    this->initBoard(board);
}

void MainWindow::connectBoard(BoardPtr board)
{
    QObject::connect(board.get(), SIGNAL(onLogin(BoardPtr, StringMap)),this, SLOT(loginEvent(BoardPtr, StringMap)));
    QObject::connect(board.get(), SIGNAL(onGetThreads(BoardPtr, ForumPtr)), this, SLOT(getThreadsHandler(BoardPtr, ForumPtr)));
    QObject::connect(board.get(), SIGNAL(onGetPosts(BoardPtr, ThreadPtr)), this, SLOT(getPostsHandler(BoardPtr, ThreadPtr)));
    QObject::connect(board.get(), SIGNAL(onGetUnreadForums(BoardPtr, ForumList)), this, SLOT(getUnreadForumsEvent(BoardPtr, ForumList)));
    QObject::connect(board.get(), SIGNAL(onMarkedForumRead(BoardPtr, ForumPtr)), this, SLOT(markForumReadHandler(BoardPtr, ForumPtr)));
    QObject::connect(board.get(), SIGNAL(onNewThread(BoardPtr, ThreadPtr)), this, SLOT(newThreadHandler(BoardPtr, ThreadPtr)));
    QObject::connect(board.get(), SIGNAL(onNewPost(BoardPtr, PostPtr)), this, SLOT(newPostHandler(BoardPtr, PostPtr)));

    QObject::connect(board.get(), &Board::onRequestError, this,
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

// called from theadslist double-click handler and
// updates the UI
void MainWindow::updateSelectedThread(ThreadPtr t)
{
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
            const auto& threads = parent->getThreads();
            for (const auto& th : threads)
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

void MainWindow::createDebugMenu()
{
    QMenu* debugMenu = this->menuBar()->addMenu("&Debug");

    {
        QAction* action = debugMenu->addAction("&Show Splash Screen");

        QObject::connect(action, &QAction::triggered, this,
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
        QObject::connect(action, &QAction::triggered, this,
            []()
            {
                const auto parsersFolder = SettingsObject().read("parsers.path").toString();
                owl::openFolder(parsersFolder);
            });
    }

    debugMenu->addSeparator();

    // Display a dialog used to write posts, for quick testing
    {
        QAction* action = debugMenu->addAction("&Show PostTextEditor dialog");
        QObject::connect(action, &QAction::triggered, this,
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
            QObject::connect(settings, &QAction::triggered, this,
                [this]()
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
            QObject::connect(quit, &QAction::triggered, this, [this]() { close(); });
        }
    }
    
    // Edit menu
    {
        QMenu* menu = menuBar()->addMenu("&Edit");
        
        auto undo = menu->addAction("Undo");
        undo->setShortcut(QKeySequence("Ctrl+Z"));
        QObject::connect(undo, &QAction::triggered, this,
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
        QObject::connect(redo, &QAction::triggered, this,
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
        
        QObject::connect(menu, &QMenu::aboutToShow, this,
            [=,this]()
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
            QObject::connect(action, &QAction::triggered, this,
                [this,action]()
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
    }
    
    // Help Menu
    {
        QMenu* helpMenu = this->menuBar()->addMenu("&Help");

        {
            QAction* action = helpMenu->addAction(tr("Documentation"));
            action->setShortcut(QKeySequence::HelpContents);
            QObject::connect(action, &QAction::triggered, this,
                [this]()
                {
                    QUrl url("http://wiki.owlclient.com");
                    _logger->trace("Launching browser: {}", url.toString().toStdString());
                    QDesktopServices::openUrl(url);
                });
        }

        {
            QAction* action = helpMenu->addAction(tr("Release Notes"));
            QObject::connect(action, &QAction::triggered, this,
            [this]()
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
            QObject::connect(action, &QAction::triggered, this,
                [this]()
                {
                    auto urlStr = QString("https://www.paypal.me/zethon");
                    QUrl url(urlStr);
                    _logger->trace("Launching browser: {}", url.toString().toStdString());
                    QDesktopServices::openUrl(url);
                });
        }

        {
            QAction* action = helpMenu->addAction(tr("Github"));
            QObject::connect(action, &QAction::triggered, this,
            [this]()
            {
                QUrl url("https://github.com/zethon/Owl");
                _logger->trace("Launching browser: {}", url.toString().toStdString());
                QDesktopServices::openUrl(url);
            });
        }

        {
            QAction* action = helpMenu->addAction(tr("&Report an issue"));
            QObject::connect(action, &QAction::triggered, this,
                [this]()
                {
                    QUrl url("https://github.com/zethon/Owl/issues");
                    _logger->trace("Launching browser: {}", url.toString().toStdString());
                    QDesktopServices::openUrl(url);

                });
        }

        helpMenu->addSeparator();

#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
        {
            QAction* action = helpMenu->addAction(tr("Show Log Folder"));
            QObject::connect(action, &QAction::triggered, this,
                [&]()
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

            QObject::connect(action, &QAction::triggered, this,
                [this]()
                {
                    auto about = new AboutDlg(this);
                    QObject::connect(about, &QDialog::finished, [about](int) { about->deleteLater(); });
                    about->resize(this->size());
                    about->open();
                });
        }
    }

#ifndef RELEASE
    createDebugMenu();
#endif

}

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

void MainWindow::createBoardPanel()
{
    // NEW
#ifdef Q_OS_MACX
    connectionView->setAttribute(Qt::WA_MacShowFocusRect, 0);
#endif

    QObject::connect(connectionView, &BoardIconView::onBoardClicked, this,
        [this](owl::BoardWeakPtr bwp)
        {
            forumContentView->doShowLoading(bwp);
            forumNavigationView->doBoardClicked(bwp);
        });

    QObject::connect(connectionView, &BoardIconView::onEditBoard, this,
        [this](owl::BoardWeakPtr boardWeakPtr)
        {
            auto boardPtr = boardWeakPtr.lock();
            if (!boardPtr) OWL_THROW_EXCEPTION(owl::Exception("Invalid board"));

            EditBoardDlg* dlg = new EditBoardDlg(boardPtr, this);

            QObject::connect(dlg, &EditBoardDlg::boardSavedEvent, this,
                [this](const BoardPtr b, const StringMap&)
            {
                _logger->trace("onBoardInfoSaved({}:{})",
                    b->getDBId(), b->getName().toStdString());
            });

            QObject::connect(dlg, &QDialog::finished, [dlg](int) { dlg->deleteLater(); });
            dlg->open();
        });

    QObject::connect(connectionView, &BoardIconView::onAddNewBoard, this,
        [this]()
        {
            QuickAddDlg* addDlg = new QuickAddDlg(this);
            connect(addDlg, SIGNAL(newBoardAddedEvent(BoardPtr)), this, SLOT(onNewBoardAdded(BoardPtr)));

            connect(addDlg, &QuickAddDlg::newBoardAddedEvent, this,
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

    QObject::connect(connectionView, &BoardIconView::onDeleteBoard, this,
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

                // remove from the database
                BOARDMANAGER->deleteBoard(board);

                // clear any resources
                board.reset();
            }
        });

    QObject::connect(connectionView, &BoardIconView::onConnectBoard, this,
        [this](owl::BoardWeakPtr bwp)
        {
            BoardPtr board = bwp.lock();
            if (board)
            {
                _logger->info("Connecting board {}", board->getName().toStdString());
                board->login();
            }
        });

    QObject::connect(connectionView, &BoardIconView::onAddNewWebBrowser, this,
        [this]()
        {
            WebViewer* viewer = new WebViewer(this);
            auto x = forumTopStack->addWidget(viewer);
            forumTopStack->setCurrentIndex(x);
            forumTopStack->repaint();
        });
}
    
void MainWindow::createThreadPanel()
{
    QObject::connect(forumNavigationView, &ForumView::onForumClicked, this,
        [this](owl::ForumPtr forum)
        {
            BoardPtr board = forum->getBoard().lock();
            if (board && board->getStatus() == BoardStatus::ONLINE)
            {
                forumContentView->doShowLoading(board);
                board->requestThreadList(forum);
                board->setLastForumId(forum->getId().toInt());
            }
        });

    QObject::connect(forumNavigationView, &ForumView::onForumListLoaded, this,
        [this]() { forumContentView->doShowLogo(); });
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

    // remove the board from the database
    BOARDMANAGER->deleteBoard(b);

    if (BOARDMANAGER->getBoardCount() == 0)
    {
        update();
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

        menuBar()->setVisible(settings.value("showMenuBar").toBool());
    }
    else
    {
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
    settings.setValue("showMenuBar", menuBar()->isVisible());
}

} // namespace owl

