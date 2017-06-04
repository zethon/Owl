// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <QtGui>
#include <QtCore>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <log4qt/logger.h>
#include <Parsers/ParserManager.h>
#include <Utils/Exception.h>
#include <Utils/QThreadEx.h>
#include "Data/BoardManager.h"
#include "AboutDlg.h"
#include "BoardsModel.h"
#include "ErrorReportDlg.h"
#include "NewThreadDlg.h"
#include "QuickAddDlg.h"
#include "PostListWidget.h"
#include "AspectRatioPixmapLabel.h"
#include "ui_MainWindow.h"

#define PANERIGHT   0
#define PANEBOTTOM  1
#define PANEHIDDEN  2

namespace Ui
{
	class MainWindow;
}

namespace owl
{

class SplashScreen : public QSplashScreen
{
    Q_OBJECT
    
public:
    SplashScreen(const QPixmap & pixmap);
    
public Q_SLOTS:
    void setDoCheck(bool var) { _bDoCheck = var; }

private:
    bool _bDoCheck;
};

// uint - DB Id
typedef QHash<uint, QThreadEx*> WorkerMap;

typedef QList<QPair<QString, QString> > UrlQueryItems;

typedef std::function<void (const UrlQueryItems&)> LinkHandler;
typedef QMap<QString, LinkHandler> LinkMessageMap;

class ImageOverlay : public QWidget
{
    void newParent()
    {
        if (!parent())
        {
            return;
        }

        parent()->installEventFilter(this);
        raise();
    }

    const QRegExp imageDef = QRegExp("data:image/[a-z]+;base64,", Qt::CaseInsensitive);

public:
    ImageOverlay(QWidget * parent = nullptr)
        : QWidget{parent},
          _closeBtn{this},
          _imgLabel{this},
          _layout{this}

    {
        setAttribute(Qt::WA_NoSystemBackground);
        newParent();

        setAttribute(Qt::WA_TranslucentBackground);
        _closeBtn.setText(tr("Close"));
        QObject::connect(&_closeBtn, &QPushButton::clicked, [this]()
        {
            this->hide();
        });

        _layout.addWidget(&_closeBtn, 0, 0, Qt::AlignRight);
        _layout.addWidget(&_imgLabel, 1, 0, 1, 1, Qt::AlignCenter);

        _imgLabel.setStyleSheet("QLabel { border: 2px solid white; background-color: white; }");
        _imgLabel.setScaledContents(true);

        setLayout(&_layout);
        QWidget::setVisible(false);
    }

    void setImageFromBase64(const QString& base64)
    {
        QString temp { base64 };
        QByteArray by = QByteArray::fromBase64(temp.remove(imageDef).toUtf8());
        setImageFromBase64(by);
    }

    void setImageFromBase64(const QByteArray& array)
    {
        auto pixmap = QPixmap::fromImage(QImage::fromData(array));
        _imgLabel.setPixmap(pixmap);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p{this};
        p.fillRect(rect(), {0, 0, 0, 190});
    }

    //! Catches resize and child events from the parent widget
    bool eventFilter(QObject * obj, QEvent * ev) override
    {
        if (obj == parent())
        {
            if (ev->type() == QEvent::Resize)
            {
                resize(static_cast<QResizeEvent*>(ev)->size());
            }
            else if (ev->type() == QEvent::ChildAdded)
            {
                raise();
            }
        }

        if (ev->type() == QEvent::KeyRelease)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(ev);
            if (keyEvent->key() == Qt::Key_Escape)
            {
                this->hide();
            }
        }

        return QWidget::eventFilter(obj, ev);
    }

    void mouseReleaseEvent(QMouseEvent* e)
    {
        Q_UNUSED(e);
        this->hide();
    }

    //! Tracks parent widget changes
    bool event(QEvent* ev) override
    {
        if (ev->type() == QEvent::ParentAboutToChange)
        {
            if (parent())
            {
                parent()->removeEventFilter(this);
            }
        }
        else if (ev->type() == QEvent::ParentChange)
        {
            newParent();
        }

        return QWidget::event(ev);
    }

    void hideEvent(QHideEvent *event) override
    {
        _imgLabel.resize(0,0);
        QWidget::hideEvent(event);
    }

private:
    QPushButton                 _closeBtn;
    AspectRatioPixmapLabel      _imgLabel;
    QGridLayout                 _layout;
};

class MainWindow : public QMainWindow, public Ui::MainWindow
{
	Q_OBJECT
	LOG4QT_DECLARE_QCLASS_LOGGER

public:
    struct MenuActions
    {
        QAction* showToolbar;
        QAction* showBoardbar;
        QAction* showStatusBar;
        
        QAction* postPaneRight;
        QAction* postPaneBelow;
        QAction* postPaneFloat;
        QAction* postPaneHidden;
    };
    
    MainWindow(SplashScreen *splash, QWidget *parent = 0);
	virtual ~MainWindow() = default;

protected:

    virtual bool event(QEvent* event) override;
    
    virtual void closeEvent(QCloseEvent* event) override
    {
        logger()->debug("Closing Owl window");
        writeSettings();
        QMainWindow::closeEvent(event);
    }

private Q_SLOTS:
	void onLoaded();
	void loadBoards();

//	void onPreferences();
	void onNewBoard();

	void onLoginClicked();
    void onLinkActivated(const QString &urlStr);
	void onTreeDoubleClicked(const QModelIndex&);
    
	void expandThreadMenuPressed();
	void threadPageNumberEnterPressed();
	void threadFirstPageBtnClicked();
	void threadPrevPageBtnClicked();
	void threadNextPageBtnClicked();
	void threadLastPageBtnClicked();
    
    void newThreadBtnClicked();

	// SLOTS related to the PostView pane
	void newPostBtnClicked();
	void expandPostMenuPressed();
    void postPageNumberEnterPressed();
	void postFirstPageBtnClicked();
	void postPrevPageBtnClicked();
	void postNextPageBtnClicked();
	void postLastPageBtnClicked();

	void onBoardToolbarItemClicked(QAction*);
	//void rightScrollButtonClicked();
	//void leftScrollButtonClicked();

	void onNewBoardAdded(BoardPtr);
	
	// handlers
	void boardwareInfoEvent(BoardPtr, StringMap);
	void loginEvent(BoardPtr, StringMap);
	void getUnreadForumsEvent(BoardPtr, ForumList);

	void getForumHandler(BoardPtr, ForumPtr);
	void getThreadsHandler(BoardPtr, ForumPtr);
	void getPostsHandler(BoardPtr, ThreadPtr);
    void markForumReadHandler(BoardPtr, ForumPtr);
	void requestErrorHandler(OwlExceptionPtr);

    // handlers when a new thread or post is submitted successfully
    void newThreadHandler(BoardPtr, ThreadPtr);
    void newPostHandler(BoardPtr, PostPtr);

	// services tree handler
	void onSvcTreeClicked(const QModelIndex &);
	void onSvcTreeContextMenu(const QPoint& pnt);

	// toolbar drop down menu
    void onRefreshForum();
	void onOpenBrowserToolbar();
	void onBoardDelete();
	void onBoardDelete(BoardPtr);

	void onCopyUrl();
    
    void onForumStructureChanged(BoardPtr);
	void onDisplayOrderChanged(BoardPtr, int);

private:
	void createLinkMessages();
	void createMenus();
    void createStatusBar();
    void createSignals();
	void createBoardPanel();
	void createThreadPanel();
	void createPostPanel();

    void readSettings();
    void writeSettings();
    
	void connectBoard(BoardPtr board);

	void startThreadLoading();
	void stopThreadLoading();

	void startPostsLoading();
	void stopPostsLoading();

    void createDebugMenu();

    QMenu* createForumMenu(ForumPtr forum);

	void updateSelectedThread(ThreadPtr thread = ThreadPtr());
	void updateSelectedForum(ForumPtr forum = ForumPtr());
    
	ForumPtr getCurrentForum() const;
    
	void navigateToThreadListPage(ForumPtr forum, int iPageNumber);
    void navigateToPostListPage(ThreadPtr thread, int iPageNumber);

    int initBoard(const BoardPtr& b);
    void openPreferences();

	QMenu* _boardToolBarCtxMenu;

	BoardsModel* _svcModel;

    LinkMessageMap _linkMessageMap;
    QSqlDatabase _db;

	QuickAddDlg*            _quickAddDlg;
	ErrorReportDlg*         _errorReportDlg;				// only one error at a time?
	QList<NewThreadDlg*>    _newThreadDialogs;

	QAction* _loginButton;

	// last item selected on board/service view
	QStandardItem* _svcTreeLastItem;

	BoardPtr _toolBarSelectedBoard;

	// TODO: ensure this is a good model for mutexes
	QMutex _updateMutex;

	// map of threads for each board
	WorkerMap       _workerMap;

	SplashScreen*	_splash;
    MenuActions     _actions;
    QWidget*        _postPaneTitleBar;
	bool            _bDoneLoading = false;
    bool			_bInitialized = false;

    QSize           _servicesTreeLastSize;

    // MainWindow UI Settings that need class members
    bool            _servicePaneVisible = true;
    bool            _statusBarVisibile = true;
    uint            _postsPanePosition = PANERIGHT;

    ImageOverlay    _imageOverlay;
};

class UpdaterMutexTryLocker
{
	QMutex&		_m;
	bool		_locked;
	
public:
	UpdaterMutexTryLocker(QMutex &m) 
		: _m(m), 
		_locked(m.tryLock()) 
	{
		// do nothing
	}

	virtual ~UpdaterMutexTryLocker() 
	{ 
		if (_locked) 
		{
			_m.unlock(); 
		}
	}
	
    UpdaterMutexTryLocker(const UpdaterMutexTryLocker&) = delete;

	bool isLocked() const 
	{ 
		return _locked; 
	}
};

class BoardUpdateWorker : public QObject
{
	Q_OBJECT
	LOG4QT_DECLARE_QCLASS_LOGGER

public:

	BoardUpdateWorker(BoardPtr board)
		: _board(board)
	{
        // do nothing
	}

    virtual ~BoardUpdateWorker() = default;
    
    //BoardPtr getBoard() { return _board; }
    void setIsDone(bool var) { _isDeleted = var; }
    
Q_SIGNALS:
    void onForumStructureChanged(BoardPtr board);

protected Q_SLOTS:

	void doWork()
	{
        if (_isDeleted)
        {
            return;
        }

		UpdaterMutexTryLocker locker(_mutex);

		if (!locker.isLocked())
		{
			logger()->trace("Updater for board %1(%2) is running. Skipping this round...",
				_board->getName(), _board->getDBId());

			return;
		}

        if (_board != nullptr)
        {
            logger()->debug("doWork() for board '%1'", _board->getName());

            try
            {
                _board->updateUnread();
                checkStructureUpdate();
            }
            catch (const WebException& ex)
            {
                logger()->error("Error during BoardUpdateWorker::doWork(): %1", ex.what());
            }
        }

        QTimer::singleShot(1000 * _board->getOptions()->getInt("refreshRate"), this, SLOT(doWork()));
	};
    
    void checkStructureUpdate()
    {
        if (_isDeleted)
        {
            return;
        }

        // how long between each structure check (in seconds)
        const uint iRefreshPeriod = 60 * 60 * 24; // one day
        
        QDateTime boardTime = _board->getLastUpdate();
        if (logger()->isTraceEnabled())
        {
            logger()->trace("Board %1(%2) - last update was %3",_board->getName(), _board->getDBId(), boardTime);
        }

        if (boardTime.secsTo(QDateTime::currentDateTime()) >= iRefreshPeriod)
        {
            logger()->debug("Board %1(%2) - verifying forum structure", _board->getName(), _board->getDBId());
            
            BoardPtr savedBoard = BOARDMANAGER->loadBoard(_board->getDBId());
            ForumPtr savedRoot = savedBoard->getRoot();

            if (savedRoot != nullptr)
            {
                // update the Board::lastUpdate member
                ForumPtr root = _board->getRootStructure(false);
                if (root != nullptr)
                {
                    if (_board->getRoot()->isStructureEqual(root))
                    {
                        logger()->trace("Board %1(%2) - stored structure and online structure are the same", _board->getName(), _board->getDBId());
                    }
                    else
                    {
                        logger()->trace("Board %1(%2) - stored structure and online structure are NOT the same", _board->getName(), _board->getDBId());
                        Q_EMIT onForumStructureChanged(_board);
                    }
                    
                    _board->setLastUpdate(QDateTime::currentDateTime());
                    BOARDMANAGER->updateBoard(_board);
                }
                else if (logger()->isWarnEnabled())
                {
                    logger()->warn("Board %1(%2) - getRootStructure() returned a 'nullptr' root", _board->getName(), _board->getDBId());
                }
            }
            else if (logger()->isWarnEnabled())
            {
                logger()->warn("Board %1(%2) - loadBoard(), getRoot() returned a 'nullptr' root", _board->getName(), _board->getDBId());
            }
        }
    }

private:

	BoardPtr _board;
	QMutex	 _mutex;
    bool     _isDeleted = false;

};

class BoardMenu : public QMenu
{
	Q_OBJECT
		LOG4QT_DECLARE_QCLASS_LOGGER

public:
    BoardMenu(BoardWeakPtr b, MainWindow* parent = nullptr)
		: QMenu(parent), _board(b)
	{
		connect(this, SIGNAL(aboutToShow()), this, SLOT(onAboutToShow()));
		createMenu();
	}

    virtual ~BoardMenu() = default;

Q_SIGNALS:
    void boardInfoSaved(const BoardPtr board, const StringMap& oldValues);

private Q_SLOTS:

	void onAboutToShow();

private:
	void createMenu();

    BoardWeakPtr			_board;
    Board::BoardStatus      _lastStatus = Board::OFFLINE;
};

class TerminalExec : public QObject
{

public:
	TerminalExec(QWidget* parent = nullptr)
		: QObject(parent)
	{
	}

	void listBoards()
	{

	}

};

class Console : public QPlainTextEdit
{
	Q_OBJECT

Q_SIGNALS:
	void getData(const QByteArray &data);
	void onCommand(const QString& command);

public:
	explicit Console(QWidget *parent = 0)
        : QPlainTextEdit(parent)
	{
		document()->setMaximumBlockCount(100);
		QPalette p = palette();
		p.setColor(QPalette::Base, Qt::black);
		p.setColor(QPalette::Text, Qt::white);
        
        setFont(QFont ("Courier New", 14));
		setPalette(p);

		printPrompt();
	}

	void putData(const QByteArray &data)
	{
		putData(QString(data));
	}

	void putData(const QString& str)
	{
		insertPlainText(str);

		QScrollBar *bar = verticalScrollBar();
		bar->setValue(bar->maximum());
	}

	void setLocalEchoEnabled(bool set)
	{
		localEchoEnabled = set;
	}

	friend Console& operator<<(Console& con, const QString& string)
	{
		con.putData(string);
		return con;
	}

protected:
	virtual void keyPressEvent(QKeyEvent *e)
	{
		switch (e->key()) 
		{
			case Qt::Key_Return:
			case Qt::Key_Enter:
				Q_EMIT onCommand(_command);
				_command.clear();
				_promptIdx = 0;
				moveCursor(QTextCursor::End);
				
				*this << "\r\n";
				printPrompt();
			break;

			case Qt::Key_Backspace:
				if (_command.size() > 0)
				{
					_promptIdx--;
					_command.chop(1);
					QPlainTextEdit::keyPressEvent(e);
				}
			break;

			case Qt::Key_Left:
				if (_promptIdx > 0)
				{
					_promptIdx--;
					QPlainTextEdit::keyPressEvent(e);
				}
			break;

			case Qt::Key_Right:
				if (_promptIdx < _command.length())
				{
					_promptIdx++;
					QPlainTextEdit::keyPressEvent(e);
				}
			break;

			case Qt::Key_Delete:
				_command.remove(_promptIdx, 1);
				QPlainTextEdit::keyPressEvent(e);
			break;

			case Qt::Key_PageUp:
			case Qt::Key_PageDown:
			case Qt::Key_Home:
			case Qt::Key_End:
			case Qt::Key_Up:
			case Qt::Key_Down:
            case Qt::Key_Escape:
			break;
			
			default:
				QPlainTextEdit::keyPressEvent(e);
				if (e->text().isSimpleText())
				{
					_command.insert(_promptIdx, e->text());
					_promptIdx++;
				}
			break;
		}
	}

	virtual void mousePressEvent(QMouseEvent *e)
	{
		Q_UNUSED(e)
		setFocus();
	}

	virtual void mouseDoubleClickEvent(QMouseEvent *e)
	{
		Q_UNUSED(e)
	}

	virtual void contextMenuEvent(QContextMenuEvent *e)
	{
		Q_UNUSED(e)
	}

	virtual void printPrompt()
	{
		putData(QString("> ") + _command);
	}

private:
	bool localEchoEnabled = true;
	
	QString	_command;
	int _promptIdx = 0;
};

class TerminalDialog : public QDialog
{
	Console		_console;
	QToolBar	_toolbar;

public:
	TerminalDialog(QWidget* parent = nullptr)
		: QDialog(parent),
		_console(this),
		_toolbar(this)
	{
        _toolbar.setIconSize(QSize(16,16));

		_toolbar.addAction("Save");
		QObject::connect(&_console, &Console::onCommand, [this](const QString& command)
		{
			qDebug() << "[" << command;
		});

		QGridLayout* layout = new QGridLayout(this);
		
		layout->addWidget(&_toolbar);
		layout->addWidget(&_console);
		setLayout(layout);

        this->resize(640,360);
	}

};

} //namespace owl
