#include <QVBoxLayout>
#include <QLabel>
#include <QTreeView>

#include  <Utils/OwlLogger.h>

#include "Data/Board.h"
#include "Data/ForumTreeModel.h"

#include "ForumView.h"

#if defined(Q_WS_WIN32)
    #define BOARDNAMEFONT       21
    #define USERNAMEFONT        21
    #define TREEFONTSIZE        21
#elif defined(Q_WS_MACX)
    #define BOARDNAMEFONT       21
    #define USERNAMEFONT        21
    #define TREEFONTSIZE        21
#else
    #define BOARDNAMEFONT       16
    #define USERNAMEFONT        12
    #define TREEFONTSIZE        12
#endif

namespace owl
{

//********************************
//* ForumView
//********************************

ForumView::ForumView(QWidget* parent /* = 0*/)
    : QWidget(parent),
      _logger { owl::initializeLogger("ForumView") }
{
    // down below we set the `layout` margin to 5, which means that the
    // parent's color gets drawn in that margin area, so we have to set
    // the parent's color to match
    parent->setStyleSheet("QWidget { background-color: #594157; }");
    setStyleSheet("QWidget { background-color: #594157; border: none; }");

    _treeView = new QTreeView(this);
    _treeView->setAttribute(Qt::WA_MacShowFocusRect, false);
    _treeView->setHeaderHidden(true);
    _treeView->setItemsExpandable(false);
    _treeView->setFont(QFont(_treeView->font().family(), TREEFONTSIZE));

    _boardLabel = new QLabel(this);
    QFont font;
    font.setPointSize(BOARDNAMEFONT);
    font.setBold(true);
    font.setWeight(75);
    _boardLabel->setFont(font);
    _boardLabel->setStyleSheet("QLabel { color : white; }");

    QHBoxLayout* userLayout = new QHBoxLayout(parent);
    _userLabel = new QLabel(this);
    _userLabel->setMaximumHeight(64);
    font.setPointSize(USERNAMEFONT);
    font.setBold(false);
    _userLabel->setFont(font);
    _userLabel->setStyleSheet("QLabel { color : lightgray; }");

    _userImgLabel = new QLabel(this);
    _userImgLabel->setMaximumHeight(64);
    _userImgLabel->setMaximumWidth(16);

    userLayout->addWidget(_userImgLabel);
    userLayout->addWidget(_userLabel);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(5);
    layout->setMargin(5);

    layout->addWidget(_boardLabel);
    layout->addLayout(userLayout);
    layout->addItem(new QSpacerItem(0,15));
    layout->addWidget(_treeView);

    setLayout(layout);
}

void ForumView::doBoardClicked(const owl::BoardWeakPtr boardWeakPtr)
{
    // The user clicked on a board, so we have to:
    // * if we are not connected, attempt to connect
    // * if the forum-tree needs to be refershed, refresh it
    // * if the forum-tree is not displayed, display it

    // We want to lock the pointer to the current board and the one
    // we were just passed in and test for equality
    {
        owl::BoardPtr currentBoard = _currentBoard.lock();
        owl::BoardPtr board = boardWeakPtr.lock();

        if (board && currentBoard)
        {
            if (*board != *currentBoard)
            {
                _currentBoard = boardWeakPtr;
            }
        }
        else if (!currentBoard && board)
        {
            _currentBoard = boardWeakPtr;
        }
    }

    owl::BoardPtr currentBoard = _currentBoard.lock();
    Q_ASSERT(currentBoard);

    ForumPtr root = currentBoard->getRootStructure(false);
    ForumTreeModel* model = new ForumTreeModel{ root };

    auto oldModel = _treeView->model();
    _treeView->setModel(model);
    _treeView->expandAll();

    _boardLabel->setText(currentBoard->getName());
    _userLabel->setText(currentBoard->getUsername());
    switch (currentBoard->getStatus())
    {
        case BoardStatus::ONLINE:
            _userImgLabel->setPixmap(QPixmap(":/icons/online.png"));
        break;
        case BoardStatus::OFFLINE:
            _userImgLabel->setPixmap(QPixmap(":/icons/offline.png"));
        break;
        case BoardStatus::ERR:
            _userImgLabel->setPixmap(QPixmap(":/icons/error.png"));
        break;
    }

    if (oldModel) oldModel->deleteLater();
}

} // namespace
