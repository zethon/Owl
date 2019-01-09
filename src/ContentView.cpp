#include <QLabel>
#include <QGridLayout>
#include <QToolButton>
#include <QPushButton>

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
//* LoadingView
//********************************

LoadingView::LoadingView(QWidget *parent)
    : QWidget(parent)
{
    _iconLbl = new QLabel(this);
    _iconLbl->setPixmap(QPixmap(":/images/owl_32.png"));
    _iconLbl->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    _iconLbl->setMaximumHeight(64);

    QLabel* textlbl = new QLabel(this);
    textlbl->setAlignment(Qt::AlignBottom|Qt::AlignHCenter);
    textlbl->setMaximumHeight(64);

    QFont font { textlbl->font() };
    font.setPointSize(32);
    font.setBold(true);

    textlbl->setFont(font);
    textlbl->setText(tr("Loading..."));

    QLabel* movieLbl = new QLabel(this);
    movieLbl->setAlignment(Qt::AlignHCenter|Qt::AlignTop);

    QMovie* working = new QMovie(":/icons/loading2.gif", QByteArray(), this);
    working->setScaledSize(QSize(48,48));

    movieLbl->setMovie(working);
    working->start();

    QVBoxLayout* layout = new QVBoxLayout();

    layout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
    layout->addWidget(_iconLbl);
    layout->addSpacing(10);
    layout->addWidget(textlbl);
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
    }
    else
    {
        _iconLbl->setPixmap(QPixmap(":/images/owl_64.png"));
    }
}

//********************************
//* ThreadListContainer
//********************************

QIcon getToolIcon(const QString& resource)
{
    QPixmap pix(QSize(32,32));
    pix.fill(Qt::transparent);
    QImage totalImage = pix.toImage();

    QImage base { resource };
    QTransform transform;
    transform.scale(22, 22);
    base = base.transformed(transform, Qt::SmoothTransformation);

    QPainter p(&totalImage);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
//    p.setRenderHint(QPainter::Antialiasing, true);
    p.drawImage(0, 0, base);

    qDebug() << "totalImage: " << totalImage.size();
    qDebug() << "base      : " << base.size();

    return QIcon(QPixmap::fromImage(totalImage));
}

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

    _paginationWidget = new owl::PaginationWidget(this);
    QObject::connect(_paginationWidget, &PaginationWidget::doGotoPage,
        [this](std::uint32_t page)
        {
            if (ForumPtr forum = _currentForum.lock(); forum)
            {
                if (BoardPtr board = forum->getBoard().lock(); board)
                {
                    Q_EMIT onLoading();
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

    QVBoxLayout* forumNameLayout = new QVBoxLayout();
    forumNameLayout->setMargin(0);
    forumNameLayout->setSpacing(0);
    forumNameLayout->addWidget(_forumNameLbl);
    forumNameLayout->addWidget(_paginationWidget);

    auto stickyBtn = new QPushButton(this);
    stickyBtn->setStyleSheet("QPushButton { border: 0px; } QPushButton::hover { background-color: #eaeaea; border-radius: 8px; }");
    stickyBtn->setIcon(getToolIcon(":/icons/sticky2.png"));
    stickyBtn->setIconSize(QSize(32, 32));
    stickyBtn->setCursor(Qt::CursorShape::PointingHandCursor);
    stickyBtn->setMaximumWidth(32);

    auto newBtn = new QPushButton(this);
    newBtn->setStyleSheet("QPushButton { border: 0px; } QPushButton::hover { background-color: #eaeaea; border-radius: 8px; }");
    newBtn->setIcon(getToolIcon(":/icons/newboard.png"));
    newBtn->setIconSize(QSize(32, 32));
    newBtn->setCursor(Qt::CursorShape::PointingHandCursor);
    newBtn->setMaximumWidth(32);

    QHBoxLayout* rightHandLayout = new QHBoxLayout();
    rightHandLayout->setMargin(0);
    rightHandLayout->setSpacing(0);
    rightHandLayout->addWidget(stickyBtn);
    rightHandLayout->addWidget(newBtn);

    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->addItem(new QSpacerItem(10,0));
    topLayout->addLayout(forumNameLayout);
    topLayout->addSpacing(10);
    topLayout->addLayout(rightHandLayout);
    topLayout->addItem(new QSpacerItem(1,0));

    // `hLine` separates the top pane from the bottom pane
    QFrame* hLine = new QFrame(this);
    hLine->setFrameShape(QFrame::HLine);
    hLine->setFrameShadow(QFrame::Sunken);

    QVBoxLayout* rootLayout = new QVBoxLayout();
    rootLayout->setSpacing(0);
    rootLayout->setMargin(0);

    rootLayout->addItem(new QSpacerItem(0,5));
    rootLayout->addLayout(topLayout);
    rootLayout->addItem(new QSpacerItem(0,5));
    rootLayout->addWidget(hLine);
    rootLayout->addWidget(_threadListWidget);

    setLayout(rootLayout);
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
}

//********************************
//* PostViewContainer
//********************************

// ┌──────────────────────────────────────────────────────────────────────────────┐
// │┌──────┐┌──────────────────────────────────────────────────┐┌────────────────┐│
// ││      ││                                                  ││                ││
// ││      ││                   Thread Title                   ││                ││
// ││      ││                                                  ││                ││
// ││  <-  │└──────────────────────────────────────────────────┘│ Other Buttons  ││
// ││      │┌──────────────────────────────────────────────────┐│                ││
// ││      ││                                                  ││                ││
// ││      ││                Pagination Widget                 ││                ││
// ││      ││                                                  ││                ││
// │└──────┘└──────────────────────────────────────────────────┘└────────────────┘│
// └──────────────────────────────────────────────────────────────────────────────┘

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
    QObject::connect(_threadListContainer, &ThreadListContainer::onLoading,
        [this]() { this->setCurrentIndex(LOADING_VIEW); });

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
