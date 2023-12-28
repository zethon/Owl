#pragma once
#include <QtGui>
#include <QtCore>
#include <QtWidgets>
#include <QWebEngineView>

#include <Parsers/ParserManager.h>
#include <Utils/Exception.h>
#include <Utils/QThreadEx.h>
#include <Data/ConnectionListModel.h>
#include "Data/BoardManager.h"
#include "ui_MainWindow.h"

#include <spdlog/spdlog.h>

namespace Ui
{
	class MainWindow;
}

namespace owl
{

class ConnectionListModel;
using ConnectionListModelPtr = std::unique_ptr<ConnectionListModel>;

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

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindow(SplashScreen *splash, QWidget *parent = nullptr);
    virtual ~MainWindow() = default;

    // @TODO: is this needed for Windows?
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
    void loadConnections();

    QuickAddDlg*            _quickAddDlg = nullptr;
    ErrorReportDlg*         _errorReportDlg = nullptr; // only one error at a time?

    // TODO: ensure this is a good model for mutexes
    QMutex _updateMutex;

    // map of threads for each board
    WorkerMap       _workerMap;
    SplashScreen*   _splash = nullptr;
    std::shared_ptr<spdlog::logger>  _logger;

    std::unique_ptr<owl::ConnectionListModel>   _connectionsModel;
};

} //namespace owl
