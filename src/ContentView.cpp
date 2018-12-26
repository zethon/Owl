#include <QLabel>
#include <QGridLayout>

#include  <Utils/OwlLogger.h>

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

ContentView::ContentView(QWidget* parent /* = 0*/)
    : QStackedWidget(parent),
      _logger { owl::initializeLogger("ContentView") }
{
    parent->setStyleSheet("QWidget { background-color: white; }");

    _logoView = new LogoView(this);

    _threadListWidget = new ThreadListWidget(this);

    _postListWidget = new PostListWebView(this);


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

    qDebug() << "THREADS: " << threadList.size();

    this->setCurrentIndex(1);
}

void ContentView::doShowListOfPosts(ThreadPtr thread)
{
    this->setCurrentIndex(2);
}

} // namespace
