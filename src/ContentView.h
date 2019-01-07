#pragma once

#include <memory>
#include <QStackedWidget>

class QLabel;
class QToolButton;

namespace spdlog
{
    class logger;
}

#define LOGO_VIEW           0
#define THREADLIST_VIEW     1
#define POSTLIST_VIEW       2
#define LOADING_VIEW        3

namespace owl
{

class PaginationWidget;
class ThreadListWidget;
class PostListWebView;

class Board;
using BoardWeakPtr = std::weak_ptr<Board>;

class Forum;
using ForumPtr = std::shared_ptr<owl::Forum>;
using ForumWeakPtr = std::weak_ptr<owl::Forum>;

class Thread;
using ThreadPtr = std::shared_ptr<owl::Thread>;
using ThreadWeakPtr = std::weak_ptr<owl::Thread>;

using SpdLogPtr = std::shared_ptr<spdlog::logger>;

class LogoView : public QWidget
{
public:
    ~LogoView() = default;
    explicit LogoView(QWidget* parent = nullptr);

private:
    QLabel*  _bgImg;

};

class LoadingView : public QWidget
{
public:
    ~LoadingView() = default;
    explicit LoadingView(QWidget* parent = nullptr);

    void setBoardInfo(BoardWeakPtr board);

private:
    QLabel*     _iconLbl = nullptr;
};

class ThreadListContainer : public QWidget
{
    Q_OBJECT

public:
    ~ThreadListContainer() = default;
    ThreadListContainer(QWidget* parent = nullptr);

    void doShowThreads(ForumPtr);

Q_SIGNALS:
    void onLoading();

private:
    QLabel*                 _forumNameLbl;
    PaginationWidget*       _paginationWidget;
    ThreadListWidget*       _threadListWidget;

    owl::ForumWeakPtr       _currentForum;
};

class PostViewContainer : public QWidget
{

Q_OBJECT

public:
    ~PostViewContainer() = default;
    PostViewContainer(QWidget* parent = nullptr);

    void showPosts(ThreadPtr thread);

Q_SIGNALS:
    void onBackButtonPressed();
    void onLoading();

private:
    QLabel*                 _threadTitle;
    PaginationWidget*       _paginationWidget;
    QToolButton*            _backButton;
    PostListWebView*        _postListWidget;

    owl::ThreadWeakPtr      _currentThread;
};

class ContentView : public QStackedWidget
{

Q_OBJECT

public:
    virtual ~ContentView() = default;
    ContentView(QWidget* parent = nullptr);

    void doShowLogo();
    void doShowLoading(BoardWeakPtr board);

    void doShowListOfThreads(ForumPtr);
    void doShowListOfPosts(ThreadPtr thread);

private:

    LogoView*               _logoView;
    ThreadListContainer*    _threadListContainer;
    PostViewContainer*      _postListContainer;
    LoadingView*            _loadingView;

    owl::BoardWeakPtr       _boardWeak;
    owl::SpdLogPtr          _logger;
};

}
