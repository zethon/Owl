#include <QLabel>
#include <QGridLayout>
#include <QToolButton>

#include  <Utils/OwlLogger.h>

#include "Data/Board.h"
#include "ThreadListWidget.h"
#include "PostListWidget.h"
#include "PaginationWidget.h"

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
//* ThreadListContainer
//********************************

//static const char* strPageNumberStyle = R"(
//QLabel
//{
//    color: #587B7F;
//}
//)";

ThreadListContainer::ThreadListContainer(QWidget *parent)
    : QWidget(parent)
{
    _threadListWidget = new ThreadListWidget(this);

    _forumNameLbl = new QLabel(this);
    QFont threadFont { _forumNameLbl->font() };
    threadFont.setPointSize(16);
    threadFont.setBold(true);
    _forumNameLbl->setFont(threadFont);
    _forumNameLbl->setWordWrap(false);
    _forumNameLbl->setMinimumHeight(32);
    _forumNameLbl->setMaximumHeight(32);
//    _forumNameLbl->setStyleSheet("QLabel { border: 2px solid white; }");

//    _pageNumberLbl = new QLabel(this);
//    QFont pageNumberFont { _pageNumberLbl->font() };
//    pageNumberFont.setPointSize(10);
//    pageNumberFont.setBold(true);
//    pageNumberFont.setWeight(45);
//    _pageNumberLbl->setFont(pageNumberFont);
//    _pageNumberLbl->setMinimumHeight(14);
//    _pageNumberLbl->setMaximumHeight(14);
//    _pageNumberLbl->setStyleSheet(strPageNumberStyle);

    // TODO: investigate the issue with constructing layouts like
    // `new MyLayout(this)` vs `new MyLayout(parent)`, because it
    // only seems to work right if we use `parent`
    _paginationWidget = new owl::PaginationWidget(parent);
    QObject::connect(_paginationWidget, &PaginationWidget::doGotoPage,
        [this](std::uint32_t page)
        {
            if (ForumPtr forum = _currentForum.lock(); forum)
            {
                if (BoardPtr board = forum->getBoard().lock(); board)
                {
                    forum->setPageNumber(page);
                    board->requestThreadList(forum);
                }
                else
                {
                    // TODO: should do something here
                    Q_ASSERT(0);
                }
            }
            else
            {
                // TODO: should do something here
                Q_ASSERT(0);
            }
        });


    QVBoxLayout* forumNameLayout = new QVBoxLayout(parent);
    forumNameLayout->setMargin(0);
    forumNameLayout->setSpacing(0);
    forumNameLayout->addWidget(_forumNameLbl);
//    forumNameLayout->addWidget(_pageNumberLbl);
    forumNameLayout->addWidget(_paginationWidget);

    QHBoxLayout* topLayout = new QHBoxLayout(parent);
    topLayout->addItem(new QSpacerItem(10,0));
    topLayout->addLayout(forumNameLayout);
    topLayout->addItem(new QSpacerItem(1,0));

    // `hLine` separates the top pane from the bottom pane
    QFrame* hLine = new QFrame(this);
    hLine->setFrameShape(QFrame::HLine);
    hLine->setFrameShadow(QFrame::Sunken);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addItem(new QSpacerItem(0,5));
    layout->addLayout(topLayout);
    layout->addItem(new QSpacerItem(0,5));
    layout->addWidget(hLine);
    layout->addWidget(_threadListWidget);

    setLayout(layout);
}

void ThreadListContainer::doShowThreads(ForumPtr forum)
{
    _threadListWidget->clearList();

    auto& threadList = forum->getThreads();
    _threadListWidget->setThreadList(threadList);

    _forumNameLbl->setText(forum->getName());

//    auto pageText = QString(tr("Page %1 of %2")).arg(forum->getPageNumber()).arg(forum->getPageCount());
//    _pageNumberLbl->setText(pageText);

    std::uint32_t current = static_cast<std::uint32_t>(forum->getPageNumber());
    std::uint32_t total = static_cast<std::uint32_t>(forum->getPageCount());
    _paginationWidget->setPages(current, total);

    _currentForum = forum;
}

//********************************
//* PostViewContainer
//********************************

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
    _threadTitle->setWordWrap(false);
    _threadTitle->setMinimumHeight(32);
    _threadTitle->setMaximumHeight(32);

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
    QFontMetrics metrics(_threadTitle->font());
    const QString elidedTitle = metrics.elidedText(thread->getTitle(), Qt::ElideRight, _threadTitle->width());
    _threadTitle->setText(elidedTitle);

    _postListWidget->clear();
    _postListWidget->showPosts(thread);
    _postListWidget->scroll(0,0);
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

    _threadListContainer = new ThreadListContainer(this);

    _postListContainer = new PostViewContainer(this);
    QObject::connect(_postListContainer, &PostViewContainer::onBackButtonPressed,
        [this]()
        {
            this->setCurrentIndex(1);
        });

    this->addWidget(_logoView);
    this->addWidget(_threadListContainer);
    this->addWidget(_postListContainer);
}

void ContentView::doShowLogo()
{
    this->setCurrentIndex(0);
}

void ContentView::doShowListOfThreads(ForumPtr forum)
{
    _threadListContainer->doShowThreads(forum);

    owl::BoardWeakPtr boardWeak = forum->getBoard();
    if (auto board = boardWeak.lock(); board)
    {
        _boardWeak = boardWeak;
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
