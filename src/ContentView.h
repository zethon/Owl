#pragma once

#include <memory>
#include <QStackedWidget>

class QLabel;
class QToolButton;

namespace spdlog
{
    class logger;
}

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

class ThreadListContainer : public QWidget
{
    Q_OBJECT

public:
    ~ThreadListContainer() = default;
    ThreadListContainer(QWidget* parent = nullptr);

    void doShowThreads(ForumPtr);

private:
    QLabel*                 _forumNameLbl;
    QLabel*                 _pageNumberLbl;
    PaginationWidget*       _paginationWidget;
    ThreadListWidget*       _threadListWidget;

    owl::ForumWeakPtr      _currentForum;
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

private:
    QLabel*                 _threadTitle;
    QToolButton*            _backButton;
    PostListWebView*        _postListWidget;
};

class ContentView : public QStackedWidget
{

Q_OBJECT

public:
    virtual ~ContentView() = default;
    ContentView(QWidget* parent = nullptr);

    void doShowLogo();
    void doShowListOfThreads(ForumPtr);

private:

    LogoView*               _logoView;
    ThreadListContainer*    _threadListContainer;
    PostViewContainer*      _postListContainer;

    owl::BoardWeakPtr       _boardWeak;
    owl::SpdLogPtr          _logger;
};

}
