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
#include "PostListWidget.h"
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

class ImageOverlay : public QWidget
{
    void newParent();

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

    void mouseReleaseEvent(QMouseEvent* e) override
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
    
	void expandThreadMenuPressed();

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
    void createSignals();
    void createBoardPanel();
    void createThreadPanel();
    void createPostPanel();

    void readWindowSettings();
    void writeWindowSettings();

    void connectBoard(BoardPtr board);

    void startThreadLoading();
    void stopThreadLoading();

    void startPostsLoading();
    void stopPostsLoading();

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
    bool            _statusBarVisibile = true;
    uint            _postsPanePosition = PANERIGHT;

    SplashScreen*   _splash = nullptr;
    ImageOverlay    _imageOverlay;

    std::shared_ptr<spdlog::logger>  _logger;
    void toggleOldControls(bool toggle);
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
