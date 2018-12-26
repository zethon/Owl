#include <QLabel>
#include <QGridLayout>

#include  <Utils/OwlLogger.h>

#include "Data/Board.h"
#include "ThreadListWidget.h"
#include "PostListWidget.h"

#include "ContentView.h"

namespace owl
{

//********************************
//* LogoView
//********************************

LogoView::LogoView(QWidget *parent)
    : QWidget(parent)
{
    _bgImg = new QLabel(this);
    _bgImg->setPixmap(QPixmap(":/images/owl-bg2.png"));
    _bgImg->setAlignment(Qt::AlignCenter);

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(_bgImg, 0, 0);

    this->setLayout(layout);
}
//********************************
//* ContentView
//********************************

void ContentView::initThreadList()
{
    _threadListWidget = new ThreadListWidget(this);

    QObject::connect(_threadListWidget, &owl::ThreadListWidget::threadLoading,
        []()
        {
            qDebug() << "POSTS ARE BEING LOADED!";
        });
}

void ContentView::initPostList()
{
    _postListWidget = new PostListWebView(this);
}

ContentView::ContentView(QWidget* parent /* = 0*/)
    : QStackedWidget(parent),
      _logger { owl::initializeLogger("ContentView") }
{
    parent->setStyleSheet("QWidget { background-color: white; }");

    _logoView = new LogoView(this);

    // initializes `_threadListWidget`
    initThreadList();

    // initializes `_postListWidget`
    initPostList();

    this->addWidget(_logoView);
    this->addWidget(_threadListWidget);
    this->addWidget(_postListWidget);
}

void ContentView::doShowLogo()
{
    this->setCurrentIndex(0);
}

void ContentView::doShowListOfThreads(ForumPtr forum)
{
    _threadListWidget->clearList();

    auto& threadList = forum->getThreads();
    _threadListWidget->setThreadList(threadList);

    owl::BoardWeakPtr boardWeak = forum->getBoard();
    if (auto board = boardWeak.lock(); board)
    {
        QObject::connect(board.get(), &owl::Board::onGetPosts,
            [this](BoardPtr, ThreadPtr thread)
            {
                this->doShowListOfPosts(thread);
            });
    }

    this->setCurrentIndex(1);
}

void ContentView::doShowListOfPosts(ThreadPtr thread)
{
    _postListWidget->clear();
    _postListWidget->showPosts(thread);
    _postListWidget->scroll(0,0);
    this->setCurrentIndex(2);
}

} // namespace
