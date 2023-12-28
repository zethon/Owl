#include <QLabel>
#include <QGridLayout>
#include <QToolButton>
#include <QPushButton>
#include <QMenu>

#include "ZFontIcon/ZFontIcon.h"
#include "ZFontIcon/ZFont_fa5.h"

#include  <Utils/OwlLogger.h>

#include "Data/Board.h"

#include "ThreadListWidget.h"
#include "PostListWidget.h"
#include "PaginationWidget.h"
#include "ContentView.h"
#include "GUIConstants.h"

#if defined(Q_OS_WIN)
    #define LOADINGVIEWFONTSIZE       14
    #define FORUMNAMELABELHEIGHT      48
#elif defined(Q_OS_MAC)
    #define LOADINGVIEWFONTSIZE       32
    // #define FORUMNAMELABELHEIGHT      48
#else
    #define LOADINGVIEWFONTSIZE       32
    #define FORUMNAMELABELHEIGHT      48
#endif

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
//* LoadingView
//********************************

LoadingView::LoadingView(QWidget *parent)
    : QWidget(parent)
{
    _iconLbl = new QLabel(this);
    _iconLbl->setPixmap(QPixmap(":/images/owl_32.png"));
    _iconLbl->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    _iconLbl->setMaximumHeight(64);

    _loadingLbl = new QLabel(this);
    _loadingLbl->setAlignment(Qt::AlignBottom|Qt::AlignHCenter);
    _loadingLbl->setMaximumHeight(64);

    QFont font { _loadingLbl->font() };
    font.setPointSize(LOADINGVIEWFONTSIZE);

    _loadingLbl->setFont(font);
    _loadingLbl->setText(tr("Loading..."));

    QLabel* movieLbl = new QLabel(this);
    movieLbl->setAlignment(Qt::AlignHCenter|Qt::AlignTop);

    QMovie* working = new QMovie(":/icons/loading2.gif", QByteArray(), this);
    working->setScaledSize(QSize(48,48));

    movieLbl->setMovie(working);
    working->start();

    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
    layout->addWidget(_iconLbl);
    layout->addSpacing(10);
    layout->addWidget(_loadingLbl);
    layout->addSpacing(10);
    layout->addWidget(movieLbl);
    layout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    setLayout(layout);
}

void LoadingView::setBoardInfo(BoardWeakPtr bwp)
{
    BoardPtr board = bwp.lock();
    if (board)
    {
        QByteArray buffer(board->getFavIcon().toLatin1());
        QImage image = QImage::fromData(QByteArray::fromBase64(buffer));

        if (image.width() != 64 || image.height() != 64)
        {
            image = image.scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }

        _iconLbl->setPixmap(QPixmap::fromImage(image));

        QString lblText;
        if (auto thread = board->getCurrentThread(); thread)
        {
            lblText = QString(tr("Loading <b>%1</b>")).arg(thread->getTitle());
        }
        if (auto forum = board->getCurrentForum(); forum)
        {
            lblText = QString(tr("Loading <b>%1</b>.")).arg(forum->getName());
        }
        else
        {
            lblText = tr("Loading...");
        }

        _loadingLbl->setText(lblText);
    }
    else
    {
        _iconLbl->setPixmap(QPixmap(":/images/owl_64.png"));
    }
}

//********************************
//* ThreadListContainer
//********************************

ThreadListContainer::ThreadListContainer(QWidget *parent)
    : QWidget(parent)
{
    this->loadUI();
    this->connectWidgets();
}

constexpr auto BGCOLOR = "white";

constexpr auto THREADLIST_COINTAINER_STYLESHEET = R"x(
QWidget
{{
    background-color: {bgcolor};
}}

#forumNameLabel
{{
    padding: 0px;
    font-size: 16px;
    font-weight: bold;
    color: #101010;
}}

#topLine
{{
    color: #C5C5C5;
}}

#topFramePane
{{
    padding-right: 5px;
}}

#optionsMenu::menu-indicator
{{
    width:0px;
}}

#hashIconLbl
{{
    padding: 0px;
    padding-left: 5px;
    margin: 0px;
}}
)x";

void ThreadListContainer::loadUI()
{
    const auto ObjectStyleSheet { fmt::format(THREADLIST_COINTAINER_STYLESHEET, fmt::arg("bgcolor", BGCOLOR)) };
    this->setStyleSheet(ObjectStyleSheet.data());

    _forumNameLbl = new QLabel(this);
    _forumNameLbl->setObjectName("forumNameLabel");
    QFont threadFont { _forumNameLbl->font() };
    threadFont.setPointSize(16);
    threadFont.setBold(true);
    _forumNameLbl->setFont(threadFont);
    _forumNameLbl->setWordWrap(false);

    _paginationWidget = new owl::PaginationWidget(this);

    auto menu = new QMenu(this);
    menu->addAction("Cat");
    menu->addAction("Dog");
    menu->addAction("Turtle");

    auto stickyBtn = new QPushButton(this);
    stickyBtn->setObjectName("stickyButton");
    stickyBtn->setIcon(QIcon(ZFontIcon::icon(Fa5::FAMILY, Fa5::fa_thumbtack, QColor{"#4e5058"}, 0.9f)));
    stickyBtn->setCursor(QCursor(Qt::PointingHandCursor));
    stickyBtn->setFlat(true);
    stickyBtn->setToolTip(tr("Show or Hide Sticky Threads"));
    stickyBtn->setStyleSheet("QToolTip { background-color:#4e5058; color:white; font-size:12px; }");

    auto newThreadBtn = new QPushButton(this);
    newThreadBtn->setObjectName("newThreadButton");
    newThreadBtn->setIcon(QIcon(ZFontIcon::icon(Fa5::FAMILY, Fa5::fa_plus, QColor{"#4e5058"}, 0.9f)));
    newThreadBtn->setCursor(QCursor(Qt::PointingHandCursor));
    newThreadBtn->setFlat(true);
    newThreadBtn->setToolTip(tr("Create New Thread"));
    newThreadBtn->setStyleSheet("QToolTip { background-color:#4e5058; color:white; font-size:12px; }");

    auto optionsMenu = new QPushButton(this);
    optionsMenu->setObjectName("optionsMenu");
    optionsMenu->setIcon(QIcon(ZFontIcon::icon(Fa5::FAMILY, Fa5::fa_bars, QColor{"#4e5058"}, 0.9f)));
    optionsMenu->setFlat(true);
    optionsMenu->setMenu(menu);

    auto hashIconLbl = new QLabel(this);
    hashIconLbl->setObjectName("hashIconLbl");
    hashIconLbl->setMaximumWidth(32);
    hashIconLbl->setMinimumWidth(32);
    auto lblPixMap = QIcon(ZFontIcon::icon(Fa5::FAMILY, Fa5::fa_hashtag, QColor{"#4e5058"}, 0.9f)).pixmap(QSize{18,18});
    hashIconLbl->setPixmap(lblPixMap);

    auto topPaneFrame = new QFrame(this);
    topPaneFrame->setObjectName("topFramePane");
    topPaneFrame->setMaximumHeight(TOPFRAME_HEIGHT);
    topPaneFrame->setMinimumHeight(TOPFRAME_HEIGHT);
    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->setSpacing(0);
    topLayout->setMargin(0);
    topLayout->addWidget(hashIconLbl);
    topLayout->addWidget(_forumNameLbl);
    topLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    topLayout->addWidget(newThreadBtn);
    topLayout->addSpacing(10);
    topLayout->addWidget(stickyBtn);
    topLayout->addSpacing(10);
    topLayout->addWidget(_paginationWidget);
    topLayout->addSpacing(10);
    topLayout->addWidget(optionsMenu);
    topPaneFrame->setLayout(topLayout);

    _loadingView = new LoadingView(this);

    _container = new QStackedWidget(this);
    _container->addWidget(_loadingView);
    _container->addWidget(_threadListWidget = new ThreadListWidget(this));
    _container->setCurrentIndex(1);

    QFrame *topLine = new QFrame;
    topLine->setFrameShape(QFrame::HLine);
    topLine->setFrameShadow(QFrame::Plain);
    topLine->setObjectName("topLine");

    QVBoxLayout* rootLayout = new QVBoxLayout();
    rootLayout->setSpacing(0);
    rootLayout->setMargin(0);

    rootLayout->addWidget(topPaneFrame);
    rootLayout->addWidget(topLine);
    rootLayout->addWidget(_container);

    setLayout(rootLayout);
}

void ThreadListContainer::connectWidgets()
{
    QObject::connect(_paginationWidget, &PaginationWidget::doGotoPage,
     [this](std::uint32_t page)
     {
         if (ForumPtr forum = _currentForum.lock(); forum)
         {
             if (BoardPtr board = forum->getBoard().lock(); board)
             {
                 _loadingView->setBoardInfo(board);
                 _container->setCurrentIndex(0);

                 forum->setPageNumber(static_cast<int>(page));
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
}

void ThreadListContainer::doShowThreads(ForumPtr forum)
{
    _threadListWidget->clearList();

    auto& threadList = forum->getThreads();
    _threadListWidget->setThreadList(threadList);

    _forumNameLbl->setText(forum->getName());

    std::uint32_t current = static_cast<std::uint32_t>(forum->getPageNumber());
    std::uint32_t total = static_cast<std::uint32_t>(forum->getPageCount());
    _paginationWidget->setPages(current, total);

    _currentForum = forum;
    _container->setCurrentIndex(1);
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

    _paginationWidget = new owl::PaginationWidget(parent);
    QObject::connect(_paginationWidget, &PaginationWidget::doGotoPage,
        [this](std::uint32_t page)
        {
            if (auto thread = _currentThread.lock(); thread)
            {
                if (BoardPtr board = thread->getBoard().lock(); board)
                {
                    Q_EMIT onLoading();
                    thread->setPageNumber(static_cast<int>(page));
                    board->setCurrentThread(thread);
                    board->requestPostList(thread, ParserEnums::REQUEST_DEFAULT, true);
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

    auto newBtn = new QPushButton("New Post", this);
    newBtn->setStyleSheet("QPushButton { border: 2px solid black; }");

    auto stickyBtn = new QPushButton("Expand/Collapse", this);
    stickyBtn->setStyleSheet("QPushButton { border: 2px solid black; }");

    QHBoxLayout* topRowLayout = new QHBoxLayout();
    topRowLayout->setMargin(0);
    topRowLayout->setSpacing(0);
    topRowLayout->addWidget(_backButton);
    topRowLayout->addWidget(_threadTitle);

    QVBoxLayout* leftHandLayout = new QVBoxLayout();
    leftHandLayout->addLayout(topRowLayout);
    leftHandLayout->addWidget(_paginationWidget);

    QHBoxLayout* rightHandLayout = new QHBoxLayout();
    rightHandLayout->addWidget(stickyBtn);
    rightHandLayout->addWidget(newBtn);

    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->addLayout(leftHandLayout);
    topLayout->addLayout(rightHandLayout);

    QFrame* hLine = new QFrame(this);
    hLine->setFrameShape(QFrame::HLine);
    hLine->setFrameShadow(QFrame::Sunken);

    QVBoxLayout* rootLayout = new QVBoxLayout();
    rootLayout->setSpacing(0);
    rootLayout->setMargin(0);

    rootLayout->addItem(new QSpacerItem(0,13));
    rootLayout->addLayout(topLayout);
    rootLayout->addItem(new QSpacerItem(0,13));
    rootLayout->addWidget(hLine);
    rootLayout->addWidget(_postListWidget);

    setLayout(rootLayout);
}

void PostViewContainer::showPosts(ThreadPtr thread)
{
    QFontMetrics metrics(_threadTitle->font());
    const QString elidedTitle = metrics.elidedText(thread->getTitle(), Qt::ElideRight, _threadTitle->width());
    _threadTitle->setText(elidedTitle);

    std::uint32_t current = static_cast<std::uint32_t>(thread->getPageNumber());
    std::uint32_t total = static_cast<std::uint32_t>(thread->getPageCount());
    _paginationWidget->setPages(current, total);

    _postListWidget->clear();
    _postListWidget->showPosts(thread);
    _postListWidget->scroll(0,0);

    _currentThread = thread;
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
    QObject::connect(_postListContainer, &PostViewContainer::onLoading,
        [this]() { this->setCurrentIndex(LOADING_VIEW); });
    QObject::connect(_postListContainer, &PostViewContainer::onBackButtonPressed,
        [this]()
        {
            this->setCurrentIndex(THREADLIST_VIEW);
        });

    _loadingView = new LoadingView(this);

    this->addWidget(_logoView);
    this->addWidget(_threadListContainer);
    this->addWidget(_postListContainer);
    this->addWidget(_loadingView);
}

void ContentView::doShowLogo()
{
    this->setCurrentIndex(LOGO_VIEW);
}

void ContentView::doShowLoading(BoardWeakPtr board)
{
    this->_loadingView->setBoardInfo(board);
    this->setCurrentIndex(LOADING_VIEW);
}

void ContentView::doShowListOfThreads(ForumPtr forum)
{
    _threadListContainer->doShowThreads(forum);

    owl::BoardWeakPtr boardWeak = forum->getBoard();
    if (auto board = boardWeak.lock(); board)
    {
        _boardWeak = boardWeak;
        QObject::connect(board.get(), &owl::Board::onGetPosts,
            [this](BoardPtr board, ThreadPtr thread)
            {
                Q_UNUSED(board);
                this->doShowListOfPosts(thread);
            });
    }

    this->setCurrentIndex(THREADLIST_VIEW);
}

void ContentView::doShowListOfPosts(ThreadPtr thread)
{
    _postListContainer->showPosts(thread);
    setCurrentIndex(POSTLIST_VIEW);
}

} // namespace
