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

class ThreadListWidget;
class PostListWebView;

class Forum;
using ForumPtr = std::shared_ptr<owl::Forum>;

class Thread;
using ThreadPtr = std::shared_ptr<owl::Thread>;

using SpdLogPtr = std::shared_ptr<spdlog::logger>;

class LogoView : public QWidget
{
public:
    ~LogoView() = default;
    explicit LogoView(QWidget* parent = nullptr);

private:
    QLabel*  _bgImg;

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
    void initThreadList();
    void initPostList();

    LogoView*               _logoView;
    ThreadListWidget*       _threadListWidget;
    PostViewContainer*      _postListContainer;

    owl::SpdLogPtr          _logger;
};

}
