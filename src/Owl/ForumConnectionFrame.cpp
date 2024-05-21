#include <QFrame>
#include <QHBoxLayout>

#include "ForumView.h"
#include "ContentView.h"
#include "ForumConnectionFrame.h"

namespace owl
{

ForumConnectionFrame::ForumConnectionFrame(owl::BoardPtr board, QWidget *parent)
    : ConnectionFrame{board->uuid(), parent}, _board{board}
{
    setupUI();
    setupBoardSignals();
    setupSignals();

    _forumContentView->doShowLogo();

    if (auto b = _board.lock(); b)
    {
        // _forumContentView->doShowLoading(b);
        // _forumNavigationView->doBoardClicked(b);
    }
    else
    {
        qDebug() << "Invalid Board Pointer";
    }
}

void ForumConnectionFrame::setupUI()
{
    auto forumTopLayout = new QHBoxLayout(this);
    forumTopLayout->setSpacing(0);
    forumTopLayout->setContentsMargins(0, 0, 0, 0);
    auto navigationFrame = new QFrame(this);
    navigationFrame->setMinimumSize(QSize(250, 0));
    navigationFrame->setMaximumSize(QSize(250, 16777215));
    navigationFrame->setStyleSheet(QString::fromUtf8("QFrame { background: yellow; }"));
    auto horizontalLayout_2 = new QHBoxLayout(navigationFrame);
    horizontalLayout_2->setSpacing(0);
    horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
    _forumNavigationView = new owl::ForumView(navigationFrame);
    QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Expanding);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(_forumNavigationView->sizePolicy().hasHeightForWidth());
    _forumNavigationView->setSizePolicy(sizePolicy1);
    _forumNavigationView->setMinimumSize(QSize(250, 0));
    _forumNavigationView->setMaximumSize(QSize(250, 16777215));
    _forumNavigationView->setAcceptDrops(false);
    horizontalLayout_2->addWidget(_forumNavigationView);
    forumTopLayout->addWidget(navigationFrame);
    auto contentFrame = new QFrame(this);
    contentFrame->setStyleSheet(QString::fromUtf8("QFrame { background: red; }"));
    auto verticalLayout = new QVBoxLayout(contentFrame);
    verticalLayout->setSpacing(0);
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    _forumContentView = new owl::ContentView(contentFrame);
    sizePolicy1.setHeightForWidth(_forumContentView->sizePolicy().hasHeightForWidth());
    _forumContentView->setSizePolicy(sizePolicy1);
    _forumContentView->setAcceptDrops(false);
    _forumContentView->setStyleSheet(QString::fromUtf8(""));
    verticalLayout->addWidget(_forumContentView);
    forumTopLayout->addWidget(contentFrame);

    this->setLayout(forumTopLayout);
}

void ForumConnectionFrame::setupSignals()
{
    QObject::connect(_forumNavigationView, &ForumView::onForumClicked, this,
        [this](owl::ForumPtr forum)
        {
            owl::BoardPtr board = _board.lock();
            if (!board) return;
            if (board && board->getStatus() == BoardStatus::ONLINE)
            {
                _forumContentView->doShowLoading(board);
                board->requestThreadList(forum);
                board->setLastForumId(forum->getId().toInt());
            }
        });

    QObject::connect(_forumNavigationView, &ForumView::onForumListLoaded, this,
        [this]()
        {
            _forumContentView->doShowLogo();
        });
}

void ForumConnectionFrame::setupBoardSignals()
{
    auto board = _board.lock();
    if (!board) return;

    QObject::connect(board.get(), &Board::onGetThreads, this,
        [this](BoardPtr b, ForumPtr forum) { _forumContentView->doShowListOfThreads(forum); });

    QObject::connect(board.get(), &Board::onLogin, this,
        [this](const BoardPtr, const owl::StringMap& info) { this->onLoginHandler(info); });
}

void ForumConnectionFrame::onLoginHandler(const owl::StringMap& info)
{
    // anything to do?
    auto b = _board.lock();
    if (!b) return;
    _forumContentView->doShowLoading(b);
    _forumNavigationView->doBoardClicked(b);
}

void ForumConnectionFrame::initFocus(Qt::FocusReason reason)
{
    this->ConnectionFrame::initFocus(reason);
    this->_forumContentView->setFocus(reason);
}

} // namespace owl
