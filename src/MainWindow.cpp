// Owl - www.owlclient.com
// Copyright (c) 2012-2018, Adalid Claure <aclaure@gmail.com>

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

#ifdef Q_OS_MACOS
#include <QtMac>
#endif

#ifdef Q_OS_WIN
    #define BOARDICONWIDGETWIDTH         70
    #define CENTRALWIDGETWIDTH          275
#elif defined(Q_OS_MAC)
    #define BOARDICONWIDGETWIDTH         70    
    #define CENTRALWIDGETWIDTH          250
#else
    #define BOARDICONWIDGETWIDTH         70
    #define CENTRALWIDGETWIDTH          250
#endif

namespace owl
{

//////////////////////////////////////////////////////////////////////////
// SplashScreen
//////////////////////////////////////////////////////////////////////////
SplashScreen::SplashScreen(const QPixmap & pixmap)
    : QSplashScreen(pixmap),
      _bDoCheck(true)
{
    QLabel* label = new QLabel(this);
    label->move(10, this->size().height() - 17);
    label->setText("Build " OWL_VERSION);
    label->show();
}

///////////////////////////////////////////////////////////////////////////////
// MainWindow
///////////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(SplashScreen *splash, QWidget *parent)
    : QMainWindow(parent),
      _svcModel{new BoardsModel(this)},
      _splash(splash),
      _imageOverlay{this},
      _logger(owl::initializeLogger("MainWindow"))
{
    setupUi(this);
    setDockNestingEnabled(true);
//    setUnifiedTitleAndToolBarOnMac(true);

    this->boardsViewDockWidget->setMaximumWidth(BOARDICONWIDGETWIDTH);
    this->boardsViewDockWidget->setMinimumWidth(BOARDICONWIDGETWIDTH);
    this->centralWidget()->setMaximumWidth(CENTRALWIDGETWIDTH);
    this->centralWidget()->setMinimumWidth(CENTRALWIDGETWIDTH);

    // TODO: move this to the OwlApplication class
    readSettings();
    
    // initialize the dictionaries
    SPELLCHECKER->init();

#ifdef Q_OS_WIN32
    this->setWindowIcon(QIcon(":/icons/owl_256.png"));
#endif
    
    setWindowTitle(QStringLiteral(APP_NAME));
    
    // TODO: the appToolbar has been made invisible for release 1.0 but maybe it will
    //		 come back for later versions
    //createToolbar();
    appToolBar->setVisible(false);
    
    // create a blank title bar for the post view dock
    postViewDockWidget->setTitleBarWidget(new QWidget(postViewDockWidget));
    boardsViewDockWidget->setTitleBarWidget(new QWidget(boardsViewDockWidget));
    
    loadBoards();

    servicesTree->setModel(_svcModel);
//    servicesTree->setVisible(false);

    QTimer::singleShot(0, this, SLOT(onLoaded()));
}

void MainWindow::onLoaded()
{
    createBoardPanel();
    createThreadPanel();
    createPostPanel();

    threadLoadingImg->setMovie(new QMovie(":/images/loading_small.gif", QByteArray(), this));
    threadLoadingImg->hide();

    postsLoadingImg->setMovie(new QMovie(":/images/loading_small.gif", QByteArray(), this));
    postsLoadingImg->hide();

    updateSelectedForum();
    updateSelectedThread();

    createSignals();
    createLinkMessages();

    //loadBoards();

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

    postsWebView->resetView();
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
        servicesTree->setHasBoards(false);
    }
    else
    {
        int iErrors = 0;
        QFont boldFont(servicesTree->font());
        boldFont.setWeight(QFont::Bold);

        const auto& list = BOARDMANAGER->getBoardList();
        for (const BoardPtr& b : list)
        {
            if (initBoard(b) == 0 && b->isAutoLogin())
            {
                _logger->debug("Starting automatic login for board '{}' with user '{}'",
                    b->getName().toStdString(), b->getUsername().toStdString());

                b->login();
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

int MainWindow::initBoard(const BoardPtr& b)
{
    const uint boardIconWidth = 32;
    const uint boardIconHeight = 32;
    
    QString boardItemTemplate = owl::getResourceHtmlFile("boardItem.html");
    Q_ASSERT(!boardItemTemplate.isEmpty());
    
    int iRet = 0;

    try
    {
        if (b->isEnabled())
        {
            ParserBasePtr parser = ParserManager::instance()->createParser(b->getProtocolName(), b->getServiceUrl());
            parser->setOptions(b->getOptions());

            b->setParser(parser);
            connectBoard(b);

            _workerMap.insert(b->getDBId(), new QThreadEx());

            QStandardItem* item = _svcModel->addBoardItem(b);           
            item->setSizeHint(QSize(item->sizeHint().width(), BoardsModel::ITEMHEIGHT));

            // Used with GetModelItem()
            b->setModelItem(item);

            BoardItemDocPtr bid(new BoardItemDoc(b, boardItemTemplate));
            b->setBoardItemDocument(bid);

            // add the board to the _boardToolBar
            QByteArray buffer(b->getFavIcon().toLatin1());
            QImage image = QImage::fromData(QByteArray::fromBase64(buffer));

            // calculate the scaling factor based on wanting a 32x32 image
            qreal iXScale = (qreal)boardIconWidth / (qreal)image.width();
            qreal iYScale = (qreal)boardIconHeight / (qreal)image.height();

            // only scale the image if it's not the right size
            if (iXScale != 1 || iYScale != 1)
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
        }
    }
    catch (const owl::Exception& ex)
    {
        iRet++;

        b->setStatus(BoardStatus::ERR);

        _logger->warn("Failed to create parser of type '{}' for board '{}': {}",
            b->getProtocolName().toStdString(), b->getName().toStdString(), ex.message().toStdString());
    }

    return iRet;
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

    QObject::connect(&dlg, &PreferencesDlg::reloadThreadPanel, this,
        [this]()
        {
            this->threadListWidget->reload();
        }, Qt::DirectConnection);

    QObject::connect(&dlg, &PreferencesDlg::reloadPostPanel, this,
        [this]()
        {
            this->postsWebView->reloadView();
        }, Qt::DirectConnection);

    QObject::connect(&dlg, &PreferencesDlg::reloadBoardPanel, this,
        [this]()
        {
            this->servicesTree->reloadView();
        }, Qt::DirectConnection);

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
            if (_servicePaneVisible)
            {
                Q_ASSERT("Don't forget me!");
            }

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
    writeSettings();
    QMainWindow::closeEvent(event);
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

    this->servicesTree->reset();

    QStandardItem* boardItem = _svcModel->getBoardItem(board, false);
    if (boardItem != NULL && !servicesTree->isExpanded(boardItem->index()))
    {
        servicesTree->expandIt(boardItem->index());
        servicesTree->setCurrentIndex(boardItem->index());
        //servicesTree->selectOneBelow(boardItem->index());
        setWindowTitle(QString("%1 - %2").arg(board->getName()).arg(APP_NAME));
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

void MainWindow::loginEvent(BoardPtr b, StringMap sp)
{
    auto doc = b->getBoardItemDocument();
    QString itemText;
    QString msg;

    _logger->debug("Board '{}' login result with user '{}' was {}",
        b->getName().toStdString(),
        b->getUsername().toStdString(),
        sp.getBool("success") ? "successful" : "unsuccessful");
    
    if (sp.getBool("success"))
    {
        doc->setOrAddVar("%BOARDSTATUSIMG%", ":/icons/online.png");
        doc->setOrAddVar("%BOARDSTATUSALT%", tr("Online"));

        if (_workerMap.contains(b->getDBId()))
        {
            QThreadEx* workerThread = _workerMap.value(b->getDBId());

            BoardUpdateWorker* pWorker = new BoardUpdateWorker(b);
            pWorker->moveToThread(workerThread);

            connect(pWorker, SIGNAL(onForumStructureChanged(BoardPtr)), this, SLOT(onForumStructureChanged(BoardPtr)));
            connect(workerThread, SIGNAL(started()), pWorker, SLOT(doWork()));
            QObject::connect(workerThread, &QThread::finished, [workerThread, pWorker] ()
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

        _logger->debug(msg.toStdString());
    }
    else
    {
        doc->setOrAddVar("%BOARDSTATUSIMG%", ":/icons/error.png");
        doc->setOrAddVar("%BOARDSTATUSALT%", tr("Offline"));

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
    
    doc->reloadHtml();
    servicesTree->update();
    QMainWindow::statusBar()->showMessage(msg, 5000);
}

// invoked when a request for posts has returned
void MainWindow::getPostsHandler(BoardPtr board, ThreadPtr thread)
{
    Q_UNUSED(board);

    QMutexLocker lock(&_updateMutex);

    stopPostsLoading();

    if (thread->getPosts().size() > 0)
    {
        postsWebView->showPosts(thread);
        updateSelectedThread(thread);
    }
    else
    {
        updateSelectedThread();
    }
}

void MainWindow::getThreadsHandler(BoardPtr b, ForumPtr forum)
{
    QMutexLocker lock(&_updateMutex);

    stopThreadLoading();

    if (forum->getThreads().size() > 0)
    {
        threadListWidget->setThreadList(forum->getThreads());
    }

    updateSelectedForum(forum);
}

// SLOT: handles the SIGNAL from a Board object. Called when the board responds 
// from a request to get a (flat) list of all forums with unread posts
// NOTE: duplicates should be removed before the list is handed to 
// this function
void MainWindow::getUnreadForumsEvent(BoardPtr board, ForumList list)
{
    bool bHasUnread = false;

    ForumHash tempHash;	
    for (ForumPtr forum : list)
    {
        tempHash.insert(forum->getId(), forum);
    }
    
    QHashIterator<QString, ForumPtr> it(board->getForumHash());

    while (it.hasNext())
    {
        it.next();

        if (it.value()->getForumType() != Forum::FORUM)
        {
            continue; 
        }

        ForumPtr treeForum = board->getForumHash().value(it.key());
        QStandardItem* item = treeForum->getModelItem();

        if (item == NULL)
        {
            continue;
        }

        // set the indicator on the forum's tree item
        QFont itemFont(item->font());
        if (tempHash.contains(it.key()))
        {
            bHasUnread = true;
            itemFont.setBold(true);
            item->setIcon(QIcon(":/icons/forum_new.png"));
//			item->setForeground(QColor(Qt::darkGreen));			
        }
        else
        {
            itemFont.setBold(false);
            item->setIcon(QIcon(":/icons/forum.png"));
//			item->setForeground(QColor(Qt::black));
        }
        item->setFont(itemFont);
    }

    if (board && board->getModelItem())
    {
        if (bHasUnread)
        {
            board->getModelItem()->setForeground(QColor(Qt::darkGreen));
        }
        else
        {
            board->getModelItem()->setForeground(QColor(Qt::black));
        }
    }
    else
    {
        _logger->error("Trying to update null board item");
    }

}

void MainWindow::getForumHandler(BoardPtr b, ForumPtr parent)
{
    QStandardItem* item = _svcModel->updateForumItem(b, parent);

    // set the column span for all the rows we just updated
    for (int i = 0; i < item->rowCount(); i++)
    {
        servicesTree->setFirstColumnSpanned(i, item->index(), true);
    }

    // show everything/anything we just added
    servicesTree->expandAll();
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

void MainWindow::onNewBoardAdded(BoardPtr b)
{
    servicesTree->setHasBoards(true);

    if (initBoard(b) == 0)
    {
        b->login();
    }
}

void MainWindow::connectBoard(BoardPtr board)
{
    connect(board.get(), SIGNAL(onLogin(BoardPtr, StringMap)),this, SLOT(loginEvent(BoardPtr, StringMap)));
    connect(board.get(), SIGNAL(onGetForum(BoardPtr, ForumPtr)), this, SLOT(getForumHandler(BoardPtr, ForumPtr)));
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
    if (f->IsRoot())
    {
//		b->updateForumHash();
        b->updateUnread();
    }
    else
    {
        startThreadLoading();
        newThreadBtn->setEnabled(false);
        servicesTree->markForumRead(f);

        b->requestThreadList(f, ParserEnums::REQUEST_NOCACHE);
    }
}

void MainWindow::newThreadHandler(BoardPtr b, ThreadPtr t)
{
    ForumPtr forum = this->getCurrentForum();

    if (forum != NULL)
    {
        startThreadLoading();
        newThreadBtn->setEnabled(false);

        b->requestThreadList(forum, ParserEnums::REQUEST_NOCACHE);
    }

    QMainWindow::statusBar()->showMessage("New thread sent", 5000);
}

void MainWindow::onSvcTreeContextMenu(const QPoint& pnt)
{
    QModelIndex curIdx = servicesTree->currentIndex();
    QStandardItem* item = _svcModel->itemFromIndex(curIdx);

    if (item == nullptr)
    {
        return;
    }

    QMenu* ctxMenu = nullptr;
    
    if (item->data().canConvert<BoardWeakPtr>())
    {
        Board* boardRaw = item->data().value<BoardWeakPtr>().lock().get();
        assert(boardRaw);

        for (auto a : boardToolbar->actions())
        {
            if (a->data().canConvert<BoardWeakPtr>())
            {
                auto btBoard = a->data().value<BoardWeakPtr>().lock();
                if (btBoard)
                {
                    if (boardRaw == btBoard.get())
                    {
                        ctxMenu = a->menu();
                        break;
                    }
                }
            }
        }
    }
    else if (item->data().canConvert<ForumPtr>())
    {
        ForumPtr forum = item->data().value<ForumPtr>();
        ctxMenu = this->createForumMenu(forum);
    }
    
    if (ctxMenu != nullptr)
    {
        ctxMenu->exec(servicesTree->mapToGlobal(pnt));        
    }
}

QMenu* MainWindow::createForumMenu(ForumPtr forum)
{
    QMenu* ret = new QMenu(this);
    
    if (forum->getForumType() != Forum::LINK)
    {
        QAction* action = ret->addAction(QIcon(":icons/refresh.png"), tr("Refresh Forum"));
        action->setToolTip(tr("Refresh Selected Forums"));
        action->setData(QVariant::fromValue(forum));
        
#ifdef Q_OS_MACX
        action->setIconVisibleInMenu(false);
#endif

        connect(action, SIGNAL(triggered()), this, SLOT(onRefreshForum()));
        
        ret->addSeparator();  
    }

    {
        //QAction* action = ret->addAction(QIcon(":/icons/link.png"), tr("Copy Forum Address"));
        QAction* action = ret->addAction(tr("Copy Forum Address"));
        action->setToolTip(tr("Copy Forum Address"));
        action->setData(QVariant::fromValue(forum));
        
        connect(action, SIGNAL(triggered()), this, SLOT(onCopyUrl()));
    }

    {
        QAction* action = ret->addAction(QIcon(":/icons/link.png"), tr("Open Forum in Browser"));
        action->setToolTip(tr("Open in Browser"));
        action->setData(QVariant::fromValue(forum));
        
#ifdef Q_OS_MACX
        action->setIconVisibleInMenu(false);
#endif

        connect(action, SIGNAL(triggered()), this, SLOT(onOpenBrowserToolbar()));
    }

    if (forum->getForumType() != Forum::LINK)
    {
        ret->addSeparator();    
        QAction* action = ret->addAction(QIcon(":/icons/markforumread.png"), tr("Mark Forum Read"));
        action->setData(QVariant::fromValue(forum));

#ifdef Q_OS_MACX
        action->setIconVisibleInMenu(false);
#endif
        
        connect(action, &QAction::triggered, [this, forum]()
        {
            auto board = forum->getBoard().lock();

            if (board)
            {
                startThreadLoading();
                board->markForumRead(forum);
            }
        });
    }
    
    return ret;
}

void MainWindow::updateSelectedForum(ForumPtr f)
{
    threadNavFrame->setEnabled(f != NULL);
    postsWebView->resetView();

    if (f != NULL)
    {
        newThreadBtn->setEnabled(f->getForumType() == Forum::FORUM);
        threadPageNumEdit->setText(QString::number(f->getPageNumber()));
        threadPageNumLbl->setText(QString::number(f->getPageCount()));
        this->currentForumLbl->setText(f->getName());
    }
    else
    {
        newThreadBtn->setEnabled(false);
        this->currentForumLbl->setText(QString());
    }
}

// called from theadslist double-click handler and
// updates the UI
void MainWindow::updateSelectedThread(ThreadPtr t)
{
    postNavFrame->setEnabled(t != NULL);

    if (t != NULL)
    {
        newPostBtn->setEnabled(true);

        postPageNumEdit->setText(QString::number(t->getPageNumber()));
        postPageNumLbl->setText(QString::number(t->getPageCount()));

        currentThreadLabel->setText(t->getTitle());

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
                servicesTree->markForumRead(parent);
            }

            update();
        }
    }
    else
    {
//		postsWebView->setThreadSelected(false);
        newPostBtn->setEnabled(false);
        currentThreadLabel->setText(QString());
    }
}

void MainWindow::onLoginClicked()
{
    QModelIndexList selected = servicesTree->selectionModel()->selectedRows();

    for (QModelIndex index : selected)
    {
        QStandardItem* item = _svcModel->itemFromIndex(index);
        
        if (item->parent() == NULL)
        {
            BoardPtr b = item->data().value<BoardWeakPtr>().lock();
            b->login();
        }
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

void MainWindow::onSvcTreeClicked(const QModelIndex& selected)
{
    QStandardItem* item = _svcModel->itemFromIndex(selected);
    
    if (item != _svcTreeLastItem)
    {
        _svcTreeLastItem = item;
        if (item->data().canConvert<BoardWeakPtr>())
        {
            threadListWidget->resetView();
            postsWebView->resetView();

            servicesTree->reset();
            servicesTree->expandIt(item->index());

            updateSelectedForum();
            updateSelectedThread();

            QModelIndex targetIndex(item->index());
            servicesTree->setCurrentIndex(targetIndex);

            auto boardWeak = item->data().value<BoardWeakPtr>();
            const auto board = boardWeak.lock();
            if (board)
            {
                setWindowTitle(QString("%1 - %2").arg(board->getName()).arg(APP_NAME));
            }
        }
        else if (item->data().canConvert<ForumPtr>())
        {
            ForumPtr forum = item->data().value<ForumPtr>();
            BoardPtr board = forum->getBoard().lock();

            if (board)
            {
                if (board->getStatus() == BoardStatus::ONLINE &&
                    forum->getForumType() != Forum::LINK)
                {
                    startThreadLoading();
                    newThreadBtn->setEnabled(false);

                    updateSelectedThread();
                    board->requestThreadList(forum);
                    board->setLastForumId(forum->getId().toInt());
                }

                if (board->getStatus() == BoardStatus::OFFLINE)
                {
                    _logger->trace("{} offline", board->getName().toStdString());
                }
            }
        }
    }
}

void MainWindow::onTreeDoubleClicked(const QModelIndex& idx)
{
    QStandardItem* item = _svcModel->itemFromIndex(idx);
    
    if (item->data().canConvert<BoardWeakPtr>())
    {
        BoardPtr board = item->data().value<BoardWeakPtr>().lock();

        if (board->getStatus() != BoardStatus::ONLINE)
        {
            board->login();
            
            auto doc = board->getBoardItemDocument();
            doc->setOrAddVar("%BOARDSTATUSIMG%", ":/icons/loading.gif");
            doc->setOrAddVar("%BOARDSTATUSALT%", tr("Connecting"));
            doc->reloadHtml();
            servicesTree->update();
        }
    }
    else if (item->data().canConvert<ForumPtr>())
    {
        ForumPtr f = item->data().value<ForumPtr>();

        auto board = f->getBoard().lock();

        if (board && board->getStatus() != BoardStatus::ONLINE)
        {
            board->login();
        }
        else if (f->getForumType() == Forum::LINK)
        {
            QDesktopServices::openUrl(f->getVar("link", false));
        }
    }
}

void MainWindow::createDebugMenu()
{
    QMenu* debugMenu = this->menuBar()->addMenu("&Debug");

    {
        QAction* action = debugMenu->addAction("&Show Splash Screen");

        QObject::connect(action, &QAction::triggered, [this]()
        {
            _splash->show();
        });
    }

    {
        QAction* action = debugMenu->addAction("&Show INI Settings Folder");
        QObject::connect(action, &QAction::triggered, [this]()
        {
            const QString writePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
            const QString iniFile = QDir(writePath).absoluteFilePath("owl.ini");
            const QFileInfo fileInfo(iniFile);
            owl::openFolder(fileInfo.absolutePath());
        });
    }

    {
        QAction* action = debugMenu->addAction("&Show Show Parsers Folder");
        QObject::connect(action, &QAction::triggered, [this]()
        {
            const auto parsersFolder = SettingsObject().read("parsers.path").toString();
            owl::openFolder(parsersFolder);
        });
    }

    debugMenu->addSeparator();

    // Display a dialog used to write posts, for quick testing
    {
        QAction* action = debugMenu->addAction("&Show PostTextEditor dialog");
        QObject::connect(action, &QAction::triggered, [this]()
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
                bool toggle = !currentForumFrame->isVisible();

                servicesTree->setVisible(toggle);

                currentForumFrame->setVisible(toggle);
                threadNavFrame->setVisible(toggle);
                threadListWidget->setVisible(toggle);
                line->setVisible(toggle);
                line_3->setVisible(toggle);

                currentThreadFrame->setVisible(toggle);
                postsWebView->setVisible(toggle);
                postNavFrame->setVisible(toggle);
                line_2->setVisible(toggle);
                line_4->setVisible(toggle);
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
        QObject::connect(undo, &QAction::triggered, [this]()
         {
             auto w = QApplication::activeWindow()->focusWidget();
             
             // try to cast to a QLineEdit
             QLineEdit* le = qobject_cast<QLineEdit*>(w);
             if (le)
             {
                 le->undo();
             }
             
         });
        
        auto redo = menu->addAction("Redo");
        redo->setShortcut(QKeySequence("Shift+Ctrl+Z"));
        QObject::connect(redo, &QAction::triggered, [this]()
         {
             auto w = QApplication::activeWindow()->focusWidget();
             
             // try to cast to a QLineEdit
             QLineEdit* le = qobject_cast<QLineEdit*>(w);
             if (le)
             {
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
//             auto activeWindow = QApplication::activeWindow();
//             auto widget = activeWindow->focusWidget();
// 
//             if (widget == servicesTree || widget == threadListView)
//             {
//                 undo->setEnabled(false);
//                 redo->setEnabled(false);
//                 cut->setEnabled(false);
//                 copy->setEnabled(false);
//                 paste->setEnabled(false);
//                 selectAll->setEnabled(false);
//                 deselectAll->setEnabled(false);
//             }
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
                servicesTree->setVisible(!servicesTree->isVisible());
                
                if (servicesTree->isVisible())
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
                    if (!postViewDockWidget->isVisible())
                    {
                        postViewDockWidget->setTitleBarWidget(new QWidget(this));
                        postViewDockWidget->setVisible(true);
                    }

                    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, postViewDockWidget);
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
                    if (!postViewDockWidget->isVisible())
                    {
                        postViewDockWidget->setTitleBarWidget(new QWidget(this));
                        postViewDockWidget->setVisible(true);
                    }

                    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, postViewDockWidget);
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
                    postViewDockWidget->setVisible(false);
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
            QObject::connect(action, &QAction::triggered, [this,action]()
            {
                boardToolbar->setVisible(!boardToolbar->isVisible());
            });

            _actions.showBoardbar = action;
        }

        viewMenu->addSeparator();

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
            if (servicesTree->isVisible())
            {
                _servicesTreeLastSize = servicesTree->size();
            }
            else if (_servicesTreeLastSize.isValid())
            {
                servicesTree->resize(_servicesTreeLastSize);
            }

            servicesTree->setVisible(!servicesTree->isVisible());
        });

    QMainWindow::statusBar()->addWidget(bvbtn);
    QMainWindow::statusBar()->setMaximumHeight(20);
}

void MainWindow::createSignals()
{
    QObject::connect(actionExit, SIGNAL(triggered()), this, SLOT(close()));
    QObject::connect(servicesTree, SIGNAL(linkActivated(const QString&)), this, SLOT(onLinkActivated(const QString&)));

    QObject::connect(threadListWidget, &owl::ThreadListWidget::threadLoading, [this]()
    {
        startPostsLoading();
        newPostBtn->setEnabled(false);
    });

    auto gotoPrevPage = [this]()
    {
        auto thread = this->threadListWidget->getCurrentThread().lock();
        if (thread)
        {
            navigateToPostListPage(thread, (thread->getPageNumber() - 1));
        }
    };

    QObject::connect(postPrevPageBtn, &QPushButton::clicked, gotoPrevPage);

    auto gotoNextPage = [=]()
    {
        auto thread = this->threadListWidget->getCurrentThread().lock();
        if (thread)
        {
            navigateToPostListPage(thread, (thread->getPageNumber() + 1));
        }
    };

    QObject::connect(postNextPageBtn, &QPushButton::clicked, gotoNextPage);

    auto gotoFirstPage = [=]()
    {
        auto thread = this->threadListWidget->getCurrentThread().lock();
        if (thread)
        {
            navigateToPostListPage(thread, 1);
        }
    };

    QObject::connect(postFirstPageBtn, &QPushButton::clicked, gotoFirstPage);

    auto gotoLastPage = [=]()
    {
        auto thread = this->threadListWidget->getCurrentThread().lock();
        if (thread)
        {
            navigateToPostListPage(thread, thread->getPageCount());
        }
    };

    QObject::connect(postLastPageBtn, &QPushButton::clicked, gotoLastPage);
    
    // items in the postNavFrame
    QObject::connect(expandAllBtn, &QPushButton::clicked, [this]()
     {
         postsWebView->expandAll();
     });
    
    QObject::connect(collapseAllBtn, &QPushButton::clicked, [this]()
     {
         postsWebView->collapseAll();
     });

    QObject::connect(postsWebView, &PostListWebView::quotePost,
        [this](ThreadPtr thread, uint index)
        {
            const BoardPtr board = thread->getBoard().lock();
            if (board)
            {
                NewThreadDlg* dlg = new NewThreadDlg(thread, this);
                const auto strQuote = board->getPostQuote(thread->getPosts().at(index));
                dlg->setQuoteText(strQuote);
                dlg->setModal(false);
                dlg->show();
            }
        });

    QObject::connect(postsWebView, &PostListWebView::replyPost,
        [this](ThreadPtr thread, uint index)
        {
            Q_UNUSED(index);
            NewThreadDlg* dlg = new NewThreadDlg(thread, this);
            dlg->setModal(false);
            dlg->show();
        });

    QObject::connect(postsWebView, &PostListWebView::showImageFromString,
        [this](const QString& base64)
        {
            _imageOverlay.setImageFromBase64(base64);
            _imageOverlay.show();
        });

    QObject::connect(postsWebView, &PostListWebView::showImageFromArray,
        [this](const QByteArray& base64)
        {
            _imageOverlay.setImageFromBase64(base64);
            _imageOverlay.show();
        });
}

void MainWindow::expandPostMenuPressed()
{
    postNavFrame->setVisible(!postNavFrame->isVisible());
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
        
        startPostsLoading();
        thread->setPageNumber(iPageNumber);
        board->requestPostList(thread, ParserEnums::REQUEST_DEFAULT, true);
        postPageNumEdit->setText(QString::number(iPageNumber));
    }
}

void MainWindow::postPageNumberEnterPressed()
{
    ThreadPtr thread = this->threadListWidget->getCurrentThread().lock();
    
    if (thread && thread->getPageNumber())
    {
        QString strText(postPageNumEdit->text());
        bool bOk = false;
        int iPageNumber = strText.toInt(&bOk);
        
        if (bOk)
        {
            navigateToPostListPage(thread, iPageNumber);
        }
        else
        {
            postPageNumEdit->setText(QString::number(thread->getPageNumber()));
        }
    }
}

void MainWindow::postFirstPageBtnClicked()
{
    ThreadPtr thread = this->threadListWidget->getCurrentThread().lock();
    
    if (thread)
    {
        navigateToPostListPage(thread, 1);
    }
}

void MainWindow::postPrevPageBtnClicked()
{
    ThreadPtr thread = this->threadListWidget->getCurrentThread().lock();

    if (thread)
    {
        navigateToPostListPage(thread, (thread->getPageNumber() - 1));
    }
}

void MainWindow::postNextPageBtnClicked()
{
    ThreadPtr thread = this->threadListWidget->getCurrentThread().lock();

    if (thread)
    {
        navigateToPostListPage(thread, (thread->getPageNumber() + 1));
    }
}

void MainWindow::postLastPageBtnClicked()
{
    ThreadPtr thread = this->threadListWidget->getCurrentThread().lock();

    if (thread)
    {
        navigateToPostListPage(thread, thread->getPageCount());
    }
}

void MainWindow::expandThreadMenuPressed()
{
    threadNavFrame->setVisible(!threadNavFrame->isVisible());
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

        startThreadLoading();
        threadPageNumEdit->setText(QString::number(iPageNumber));
        forum->setPageNumber(iPageNumber);
        board->requestThreadList(forum);
    }
}

void MainWindow::threadPageNumberEnterPressed()
{
    ForumPtr forum = getCurrentForum();

    if (forum != NULL && forum->getPageNumber() >= 1)
    {
        QString strText(threadPageNumEdit->text());
        bool bOk = false;
        int iPageNumber = strText.toInt(&bOk);
        
        if (bOk)
        {
            navigateToThreadListPage(forum, iPageNumber);
        }
        else
        {
            threadPageNumEdit->setText(QString::number(forum->getPageNumber()));
        }
    }
}

void MainWindow::threadFirstPageBtnClicked()
{
    ForumPtr forum = getCurrentForum();

    if (forum != NULL)
    {
        navigateToThreadListPage(forum, 1);
    }
}

void MainWindow::threadPrevPageBtnClicked()
{
    ForumPtr forum = getCurrentForum();

    if (forum != NULL)
    {
        navigateToThreadListPage(forum, (forum->getPageNumber() - 1));
    }
}

void MainWindow::threadNextPageBtnClicked()
{
    ForumPtr forum = getCurrentForum();

    if (forum != NULL)
    {
        navigateToThreadListPage(forum, (forum->getPageNumber() + 1));
    }
}

void MainWindow::threadLastPageBtnClicked()
{
    ForumPtr forum = getCurrentForum();

    if (forum != NULL)
    {
        navigateToThreadListPage(forum, forum->getPageCount());
    }
}
    
void MainWindow::newThreadBtnClicked()
{
    ForumPtr forum = this->getCurrentForum();
    if (forum != NULL)
    {
        NewThreadDlg* newdlg = new NewThreadDlg(forum, this);
        newdlg->setModal(false);
        newdlg->show();
    }
}

void MainWindow::newPostBtnClicked()
{
    auto threadPtr = this->threadListWidget->getCurrentThread().lock();
    if (threadPtr)
    {
        NewThreadDlg* dlg = new NewThreadDlg(threadPtr, this);
        dlg->setModal(false);
        dlg->show();
    }
}

// event sent from Board object notifying the UI that a new post
// has been successfully posted
void MainWindow::newPostHandler(BoardPtr b, PostPtr p)
{
    if (p)
    {
        ThreadPtr thread = this->threadListWidget->getCurrentThread().lock();

        if (thread && thread == p->getParent())
        {
            this->startPostsLoading();
            this->newPostBtn->setEnabled(false);
            b->requestPostList(thread, ParserEnums::REQUEST_NOCACHE);
        }
        else
        {
            Q_ASSERT(p->getParent() != nullptr);
            QString msg = QString("New post successfully sent in thread '%1'").arg(p->getParent()->getTitle());
            QMainWindow::statusBar()->showMessage(msg, 5000);
        }
    }
    else
    {
        _logger->warn("Null new post returned for board {} ({})",
            b->getName().toStdString(), b->getDBId());

        QMainWindow::statusBar()->showMessage("New post saved", 5000);
    }
}

ForumPtr MainWindow::getCurrentForum() const
{
    ForumPtr forum;
    QModelIndex index = servicesTree->currentIndex();
    QStandardItem* item = _svcModel->itemFromIndex(index);

    if (item != NULL && item->data().canConvert<ForumPtr>())
    {
        forum = item->data().value<ForumPtr>();
    }

    return forum;
}

void MainWindow::createLinkMessages()
{
    _linkMessageMap.insert("boardconfig", std::bind(&MainWindow::onNewBoard, this));
}

void MainWindow::createBoardPanel()
{
    // NEW
#ifdef Q_OS_MACX
    servicesTree2->setAttribute(Qt::WA_MacShowFocusRect, 0);
#endif

    QObject::connect(servicesTree2, &BoardIconView::onBoardClicked,
        [this](owl::BoardWeakPtr bwp) { threadListWidget2->doBoardClicked(bwp); });

    QObject::connect(servicesTree2, &BoardIconView::onEditBoard,
        [this](owl::BoardWeakPtr boardWeakPtr)
        {
            auto boardPtr = boardWeakPtr.lock();
            if (!boardPtr) OWL_THROW_EXCEPTION(owl::Exception("Invalid board"));

            EditBoardDlg* dlg = new EditBoardDlg(boardPtr, this);

            QObject::connect(dlg, &QDialog::finished, [dlg](int) { dlg->deleteLater(); });
            QObject::connect(dlg, &EditBoardDlg::boardSavedEvent,
                [this](const BoardPtr b, const StringMap&)
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

            dlg->open();
        });

    QObject::connect(servicesTree2, &BoardIconView::onAddNewBoard,
        [this]()
        {
            QuickAddDlg* addDlg = new QuickAddDlg(this);
            QObject::connect(addDlg, &QDialog::finished, [addDlg](int) { addDlg->deleteLater(); });
            addDlg->open();
        });

    // OLD
#ifdef Q_OS_MACX
    servicesTree->setAttribute(Qt::WA_MacShowFocusRect, 0);
    servicesTree2->setAttribute(Qt::WA_MacShowFocusRect, 0);
#endif
    
    servicesTree->collapseAll();
    servicesTree->setSelectionBehavior(QTreeView::SelectRows);
    servicesTree->setSortingEnabled(false);
    servicesTree->resizeColumnToContents(0);
    servicesTree->setColumnWidth(0,servicesTree->width() * 0.66);
    servicesTree->setContextMenuPolicy(Qt::CustomContextMenu);
    //servicesTree->setModel(_svcModel);
    
    auto itemDelegate = new BoardItemDelegate();
    servicesTree->setItemDelegate(itemDelegate);
    
    connect(servicesTree, 
        SIGNAL(clicked(const QModelIndex &)), 
        this, 
        SLOT(onSvcTreeClicked(const QModelIndex &)));

    connect(servicesTree, 
        SIGNAL(customContextMenuRequested(const QPoint&)), 
        this, 
        SLOT(onSvcTreeContextMenu(const QPoint &)));
    
    connect(servicesTree,
        SIGNAL(doubleClicked(const QModelIndex&)),
        this,
        SLOT(onTreeDoubleClicked(const QModelIndex&)));
}
    
void MainWindow::createThreadPanel()
{
    QObject::connect(threadListWidget2, &ForumView::onForumClicked,
        [](owl::ForumPtr forum)
        {
            qDebug() << "SELECTED FORUM: " << forum->getName();
        });

    // the "New Thread" button is disabled on startup
    newThreadBtn->setEnabled(false);

    // set up the menu's expand-button
    auto moreMenuPressed = [this]
    {
        threadNavFrame->setVisible(!threadNavFrame->isVisible());
        line->setVisible(threadNavFrame->isVisible());
    };

    QObject::connect(threadMenuMoreBtn, &QPushButton::clicked, moreMenuPressed);
    QObject::connect(currentForumLbl, &ClickableLabel::clicked, moreMenuPressed);

    // and now connect all the other buttons
    QObject::connect(newThreadBtn, SIGNAL(clicked()), this, SLOT(newThreadBtnClicked()));
    QObject::connect(threadPrevPageBtn, SIGNAL(clicked()), this, SLOT(threadPrevPageBtnClicked()));
    QObject::connect(threadNextPageBtn, SIGNAL(clicked()), this, SLOT(threadNextPageBtnClicked()));
    QObject::connect(threadFirstPageBtn, SIGNAL(clicked()), this, SLOT(threadFirstPageBtnClicked()));
    QObject::connect(threadLastPageBtn, SIGNAL(clicked()), this, SLOT(threadLastPageBtnClicked()));
    QObject::connect(threadPageNumEdit, SIGNAL(returnPressed()), this, SLOT(threadPageNumberEnterPressed()));

    QObject::connect(stickyButton, &QToolButton::clicked,
        [this](bool checked)
        {
            this->threadListWidget->setShowStickies(checked);
            this->threadListWidget->refreshThreadDisplay();

            if (checked)
            {
                stickyButton->setToolTip(tr("Click to hide sticky threads"));
            }
            else
            {
                stickyButton->setToolTip(tr("Click to hide sticky threads"));
            }
        });
}

void MainWindow::createPostPanel()
{
    // the "New Post" button is disabled by default
    newPostBtn->setEnabled(false);

    // set up the menu's expand button
    auto moreMenuPressed = [this]
    {
            postNavFrame->setVisible(!postNavFrame->isVisible());
            line_2->setVisible(postNavFrame->isVisible());
    };

    QObject::connect(postMenuMoreBtn, &QPushButton::clicked, moreMenuPressed);
    QObject::connect(currentThreadLabel, &ClickableLabel::clicked, moreMenuPressed);

    // set up the rest of the view's connections
    QObject::connect(postPageNumEdit, SIGNAL(returnPressed()), this, SLOT(postPageNumberEnterPressed()));
    QObject::connect(newPostBtn, SIGNAL(clicked()), this, SLOT(newPostBtnClicked()));

    QObject::connect(postViewDockWidget, &QDockWidget::visibilityChanged, 
        [this](bool bVisible)
    {
        this->_actions.postPaneHidden->setChecked(!bVisible);
    });
}

void MainWindow::onRefreshForum()
{
    QAction* caller = qobject_cast<QAction*>(sender());
    
    ForumPtr forum;
    if (caller != NULL && !caller->data().canConvert<ForumPtr>())
    {
        forum = caller->data().value<ForumPtr>();
    }
    else
    {
        forum = getCurrentForum();
    }
    
    if (forum != NULL)
    {
        auto board = forum->getBoard().lock();

        if (board)
        {
            startThreadLoading();
            newThreadBtn->setEnabled(false);

            board->requestThreadList(forum, ParserEnums::REQUEST_NOCACHE);
        }
    }
}
    
// Invoked from the board toolbar when user clicks 'Open in Browser'
void MainWindow::onOpenBrowserToolbar()
{
    QAction* caller = qobject_cast<QAction*>(sender());

    if (caller != NULL)
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

    if (caller != NULL)
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

    if (caller != NULL && caller->data().canConvert<BoardWeakPtr>())
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
    QThread* thread = _workerMap.value(b->getDBId());
    thread->exit();

    _workerMap.remove(b->getDBId());

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

    // clear the serviceTree selection and remove
    // the board from the serviceTree
    servicesTree->clearSelection();
    _svcModel->removeBoardItem(b);

    // remove the board from the database
    BOARDMANAGER->deleteBoard(b);
    //BOARDMANAGER->reload();

    if (BOARDMANAGER->getBoardCount() == 0)
    {
        servicesTree->setHasBoards(false);
        update();

        updateSelectedForum(ForumPtr());
        updateSelectedThread(ThreadPtr());

        setWindowTitle(QStringLiteral(APP_NAME));
    }

    b.reset();
}
    
void MainWindow::readSettings()
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

        _servicePaneVisible = settings.value("servicePaneVisibility").toBool();
        servicesTree->setVisible(_servicePaneVisible);

        _statusBarVisibile = settings.value("statusBarVisible").toBool();
        _postsPanePosition = settings.value("postsPanePosition").toUInt();

        threadListWidget->setShowStickies(settings.value("showStickies").toBool());
        stickyButton->setChecked(settings.value("showStickies").toBool());
        if (settings.value("showStickies").toBool())
        {
            stickyButton->setToolTip(tr("Click to hide sticky threads"));
        }
        else
        {
            stickyButton->setToolTip(tr("Click to hide sticky threads"));
        }
    }
    else
    {
        _logger->info("No settings file found at '{}', using defaults", iniFile.toStdString());
    }
}

void MainWindow::writeSettings()
{
    const QString writePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    const QString iniFile = QDir(writePath).absoluteFilePath("owl.ini");

    QSettings settings(iniFile, QSettings::IniFormat);
    settings.beginGroup("MainWindow");

    settings.setValue("geometry",saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("statusBarGeometry", QMainWindow::statusBar()->saveGeometry());
    settings.setValue("servicePaneVisibility", servicesTree->isVisible());
    settings.setValue("statusBarVisible", _statusBarVisibile);
    settings.setValue("postsPanePosition", _postsPanePosition);
    settings.setValue("showStickies", threadListWidget->showStickies());
}
                                                             
void MainWindow::startThreadLoading()
{
    threadNavFrame->setEnabled(false);
//    threadPageNav->setEnabled(false);

    threadLoadingImg->show();
    threadLoadingImg->movie()->start();
    currentForumLbl->setText("Loading...");
}

void MainWindow::stopThreadLoading()
{
    threadNavFrame->setEnabled(true);
//    threadPageNav->setEnabled(true);

    threadLoadingImg->hide();
    threadLoadingImg->movie()->stop();
    currentForumLbl->setText("");
}

void MainWindow::startPostsLoading()
{
    postNavFrame->setEnabled(false);
    postsLoadingImg->show();
    postsLoadingImg->movie()->start();
    currentThreadLabel->setText("Loading...");
}

void MainWindow::stopPostsLoading()
{
    postNavFrame->setEnabled(true);
    postsLoadingImg->hide();
    postsLoadingImg->movie()->stop();
    currentThreadLabel->setText("");
}

void MainWindow::onDisplayOrderChanged(BoardPtr b, int iDirection)
{	
    QMutexLocker locker(&_updateMutex);

    // clear out any selection in the servicesTree the user may have
    servicesTree->setCurrentIndex(QModelIndex());

    // rearrange the board in the _svcModel 
    auto boardItem = _svcModel->getBoardItem(b);
    auto row = boardItem->row();
    
    auto takenBoardRow = _svcModel->takeRow(row);
    auto takenSepRow = _svcModel->takeRow(row);

    auto newRow = row + (iDirection * 2);
    _svcModel->insertRow(newRow, takenSepRow);
    _svcModel->insertRow(newRow, takenBoardRow);

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
        if (iDirection > 0)
        {
            auto beforeIdx = actionIdx - +iDirection + 1;
            auto numActions = boardToolbar->actions().size();
            
            if (beforeIdx >= numActions)
            {
                boardToolbar->addAction(tbAction);
            }
            else
            {
                boardToolbar->removeAction(tbAction);
                
                // iDirection + 1 since we have to insert 'before'
                auto otherIdx = row + iDirection + 1;
                if (otherIdx >= boardToolbar->actions().size())
                {
                    // board icon is being moved to last position
                    boardToolbar->addAction(tbAction);
                }
                else
                {
                    QAction* otherAction = boardToolbar->actions().at(otherIdx);
                    boardToolbar->insertAction(otherAction, tbAction);
                }
            }
        }
        else if (iDirection < 0)
        {
            auto otherIdx = actionIdx - 1;
            QAction* otherAction = boardToolbar->actions().at(otherIdx);

            if (otherAction)
            {
                boardToolbar->removeAction(tbAction);
                boardToolbar->insertAction(otherAction, tbAction);
            }
        }
    }
    else
    {
        _logger->warn("Board '{}' ({}) not found in boardToolbar",
            b->getName().toStdString(), b->getDBId());
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

} // namespace owl

