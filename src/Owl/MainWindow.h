#pragma once
#include <QtGui>
#include <QtCore>
#include <QtWidgets>
#include <Parsers/ParserManager.h>
#include <Utils/Exception.h>
#include <Utils/QThreadEx.h>
#include "Data/BoardManager.h"
#include "NewThreadDlg.h"
#include "AspectRatioPixmapLabel.h"
#include "ui_MainWindow.h"

#include <spdlog/spdlog.h>

#define PANERIGHT   0
#define PANEBOTTOM  1
#define PANEHIDDEN  2

namespace Ui
{
	class MainWindow;
}

namespace owl
{

class ErrorReportDlg;
class QuickAddDlg;

// uint - DB Id
typedef QHash<std::size_t, QThreadEx*> WorkerMap;

typedef QList<QPair<QString, QString> > UrlQueryItems;

typedef std::function<void (const UrlQueryItems&)> LinkHandler;
typedef QMap<QString, LinkHandler> LinkMessageMap;

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

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

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
    
    MainWindow(SplashScreen *splash, QWidget *parent = nullptr);
    virtual ~MainWindow() = default;

    void showMenuBar(bool visible) const;

protected:

    virtual bool event(QEvent* event) override;
    virtual void closeEvent(QCloseEvent* event) override;
    virtual bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;

private Q_SLOTS:
	void onLoaded();
	void loadBoards();

//	void onPreferences();
	void onNewBoard();

    void onLinkActivated(const QString &urlStr);


	void onBoardToolbarItemClicked(QAction*);
	//void rightScrollButtonClicked();
	//void leftScrollButtonClicked();

	void onNewBoardAdded(BoardPtr);
	
	// handlers
	void boardwareInfoEvent(BoardPtr, StringMap);
    void loginEvent(BoardPtr, const StringMap&);
	void getUnreadForumsEvent(BoardPtr, ForumList);

	void getThreadsHandler(BoardPtr, ForumPtr);
	void getPostsHandler(BoardPtr, ThreadPtr);
    void markForumReadHandler(BoardPtr, ForumPtr);

    // handlers when a new thread or post is submitted successfully
    void newThreadHandler(BoardPtr, ThreadPtr);
    void newPostHandler(BoardPtr, PostPtr);

	// toolbar drop down menu
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
    void createBoardPanel();
    void createThreadPanel();
    void createPostPanel();

    void readWindowSettings();
    void writeWindowSettings();

    void connectBoard(BoardPtr board);

//    void startThreadLoading();
//    void stopThreadLoading();

    void createDebugMenu();

    void updateSelectedThread(ThreadPtr thread = ThreadPtr());
    void updateSelectedForum(ForumPtr forum = ForumPtr());

    void navigateToThreadListPage(ForumPtr forum, int iPageNumber);
    void navigateToPostListPage(ThreadPtr thread, int iPageNumber);

    bool initBoard(const BoardPtr& b);
    void openPreferences();

    QMenu* _boardToolBarCtxMenu = nullptr;

    LinkMessageMap _linkMessageMap;

    QuickAddDlg*            _quickAddDlg = nullptr;
    ErrorReportDlg*         _errorReportDlg = nullptr;				// only one error at a time?
    QList<NewThreadDlg*>    _newThreadDialogs;

    QAction* _loginButton = nullptr;

    // last item selected on board/service view
    QStandardItem* _svcTreeLastItem = nullptr;

    BoardPtr _toolBarSelectedBoard;

    // TODO: ensure this is a good model for mutexes
    QMutex _updateMutex;

    // map of threads for each board
    WorkerMap       _workerMap;


    MenuActions     _actions;
    QWidget*        _postPaneTitleBar = nullptr;
    bool            _bDoneLoading = false;
    bool			_bInitialized = false;

    QSize           _servicesTreeLastSize;

    // MainWindow UI Settings that need class members
    bool            _statusBarVisibile = false;
    uint            _postsPanePosition = PANERIGHT;

    SplashScreen*   _splash = nullptr;

    std::shared_ptr<spdlog::logger>  _logger;
};

class BoardMenu : public QMenu
{
    Q_OBJECT

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
    BoardStatus      _lastStatus = BoardStatus::OFFLINE;
};

} //namespace owl
