#include <QLabel>
#include <QGridLayout>
#include <QToolButton>

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
//* PostViewContainer
//********************************"

static const char* strBackButtonStyle = R"(
QToolButton
{
    border: 0px;
}

QToolButton:hover
{
    background-color: #D0D0D0;
    border-radius: 2px;
}
)";

owl::PostViewContainer::PostViewContainer::PostViewContainer(QWidget* parent)
    : QWidget(parent)
{
    _postListWidget = new PostListWebView(this);

    QHBoxLayout* topLayout = new QHBoxLayout(parent);
    _backButton = new QToolButton(this);
    _backButton->setIcon(QIcon(":/icons/left-arrow.png"));
    _backButton->setMinimumWidth(32);
    _backButton->setMaximumWidth(32);
    _backButton->setStyleSheet(QString::fromLatin1(strBackButtonStyle));
    QObject::connect(_backButton, &QToolButton::clicked, [this]() { Q_EMIT onBackButtonPressed(); });

    _threadTitle = new QLabel(this);
    QFont threadFont { _threadTitle->font() };
    threadFont.setPointSize(16);
    threadFont.setBold(true);
    _threadTitle->setFont(threadFont);
    _threadTitle->setWordWrap(true);

    topLayout->addItem(new QSpacerItem(1,0));
    topLayout->addWidget(_backButton);
    topLayout->addItem(new QSpacerItem(10,0));
    topLayout->addWidget(_threadTitle);
    topLayout->addItem(new QSpacerItem(1,0));

    QFrame* hLine = new QFrame(this);
    hLine->setFrameShape(QFrame::HLine);
    hLine->setFrameShadow(QFrame::Sunken);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addItem(new QSpacerItem(0,13));
    layout->addLayout(topLayout);
    layout->addItem(new QSpacerItem(0,13));
    layout->addWidget(hLine);
    layout->addWidget(_postListWidget);

    setLayout(layout);
}

void PostViewContainer::showPosts(ThreadPtr thread)
{
//    QFontMetrics metrics(_threadTitle->font());
//    const QString elidedTitle = metrics.elidedText(thread->getTitle(), Qt::ElideRight, _threadTitle->width());
    _threadTitle->setText(thread->getTitle());

    _postListWidget->clear();
    _postListWidget->showPosts(thread);
    _postListWidget->scroll(0,0);
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
    _postListContainer = new PostViewContainer(this);

    QObject::connect(_postListContainer, &PostViewContainer::onBackButtonPressed,
        [this]()
        {
            this->setCurrentIndex(1);
        });
}

ContentView::ContentView(QWidget* parent /* = 0*/)
    : QStackedWidget(parent),
      _logger { owl::initializeLogger("ContentView") }
{
    parent->setStyleSheet("QWidget { background-color: white; }");

    _logoView = new LogoView(this);

    // initializes `_threadListWidget`
    initThreadList();

    // initializes `_postListContainer`
    initPostList();

    this->addWidget(_logoView);
    this->addWidget(_threadListWidget);
    this->addWidget(_postListContainer);
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
                _postListContainer->showPosts(thread);
                setCurrentIndex(2);
            });
    }

    this->setCurrentIndex(1);
}

} // namespace
