#pragma once

#include <memory>
#include <QStackedWidget>

class QLabel;
class QTreeView;

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

class ContentView : public QStackedWidget
{

Q_OBJECT

public:
    virtual ~ContentView() = default;
    ContentView(QWidget* parent = nullptr);

    void doShowLogo();
    void doShowListOfThreads(ForumPtr);
    void doShowListOfPosts(ThreadPtr);

private:
    LogoView*               _logoView;
    ThreadListWidget*       _threadListWidget;
    PostListWebView*        _postListWidget;

    owl::SpdLogPtr          _logger;
};

}
