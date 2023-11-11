#pragma once
#include <QtGui>
#include <QtCore>
#include <QtWidgets>
#include <QWebEngineView>

#include <Parsers/ParserManager.h>
#include <Utils/Exception.h>
#include <Utils/QThreadEx.h>
#include "Data/BoardManager.h"
#include "ui_MainWindow.h"

#include <spdlog/spdlog.h>

namespace Ui
{
	class MainWindow;
}

namespace owl
{

class ErrorReportDlg;
class QuickAddDlg;

// uint - DB Id
using WorkerMap = QHash<std::size_t, QThreadEx*>;

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

class WebViewer : public QWidget
{
public:
    explicit WebViewer(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(0);
        mainLayout->setContentsMargins(0, 0, 0, 0);

        auto addressFrame = new QFrame(this);
        addressFrame->setMaximumSize(QSize(16777215, 60));
        addressFrame->setFrameShape(QFrame::StyledPanel);
        addressFrame->setFrameShadow(QFrame::Raised);
        auto horizontalLayout = new QHBoxLayout(addressFrame);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        auto lineEdit = new QLineEdit(addressFrame);
        horizontalLayout->addWidget(lineEdit);
        auto pushButton = new QPushButton(addressFrame);
        horizontalLayout->addWidget(pushButton);
        mainLayout->addWidget(addressFrame);

        auto browserFrame = new QFrame(this);
        browserFrame->setFrameShape(QFrame::StyledPanel);
        browserFrame->setFrameShadow(QFrame::Raised);
        auto verticalLayout = new QVBoxLayout(browserFrame);
        auto webEngineView = new QWebEngineView(browserFrame);
        webEngineView->setUrl(QUrl(QString::fromUtf8("https://amb.dog")));
        verticalLayout->addWidget(webEngineView);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);

        mainLayout->addWidget(browserFrame);

        this->setLayout(mainLayout);
    }

};

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindow(SplashScreen *splash, QWidget *parent = nullptr);
    virtual ~MainWindow() = default;

    void showMenuBar(bool visible) const;

protected:

    virtual void closeEvent(QCloseEvent* event) override;
    virtual bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;

private Q_SLOTS:
	void onLoaded();
	void loadBoards();
	void onNewBoard();
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
	void onBoardDelete();
	void onBoardDelete(BoardPtr);

    void onForumStructureChanged(BoardPtr);

private:
    void createMenus();
    void createBoardPanel();
    void createThreadPanel();

    void readWindowSettings();
    void writeWindowSettings();

    void connectBoard(BoardPtr board);

    void createDebugMenu();

    void updateSelectedThread(ThreadPtr thread = ThreadPtr());

    bool initBoard(const BoardPtr& b);
    void openPreferences();

    QuickAddDlg*            _quickAddDlg = nullptr;
    ErrorReportDlg*         _errorReportDlg = nullptr; // only one error at a time?

    // TODO: ensure this is a good model for mutexes
    QMutex _updateMutex;

    // map of threads for each board
    WorkerMap       _workerMap;
    SplashScreen*   _splash = nullptr;
    std::shared_ptr<spdlog::logger>  _logger;
};

} //namespace owl
