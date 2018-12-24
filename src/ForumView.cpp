#include <QVBoxLayout>
#include <QLabel>
#include <QScrollBar>

#include  <Utils/OwlLogger.h>

#include "Data/Board.h"
#include "Data/ForumTreeModel.h"

#include "ForumView.h"

#if defined(Q_OS_WIN)
    #define BOARDNAMEFONT       12
    #define USERNAMEFONT        10
    #define TREEFONTSIZE        10
#elif defined(Q_OS_MAC)
    #define BOARDNAMEFONT       18
    #define USERNAMEFONT        14
    #define TREEFONTSIZE        16
#else
    #define BOARDNAMEFONT       16
    #define USERNAMEFONT        12
    #define TREEFONTSIZE        12
#endif

static const char* strListStyleSheet = R"(
QListView
{
    background: #594157;
    border-style: none;
}

QListView::item::selected{}
QListView::item::hover
{
    background: darkgrey;
}
)";

static const char* strListViewScrollStyle = R"(
QScrollBar:vertical {
    border: 0px;
    background: transparent;
    width: 8px;
    margin: 0px 0px 0px 0px;
}
QScrollBar::handle:vertical
{
    background: darkgrey;
}
QScrollBar::add-line:vertical
{
    height: 0px;
    subcontrol-position: bottom;
    subcontrol-origin: margin;
}
QScrollBar::sub-line:vertical
{
    height: 0 px;
    subcontrol-position: top;
    subcontrol-origin: margin;
}
)";

namespace owl
{

//********************************
//* ForumViewDelegate
//********************************

void ForumViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}

QSize ForumViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize retsize { option.rect.size() };
    retsize.setHeight(30);

//    if (index.model()->hasIndex(index.row() + 1, index.column()))
//    {
//        auto nextIdx = index.model()->index(index.row() + 1, index.column());
//        owl::Forum* item = static_cast<owl::Forum*>(nextIdx.internalPointer());
//        if (item->getForumType() == owl::Forum::ForumType::CATEGORY)
//        {
//            retsize.setHeight(100);
//        }
//    }

    return retsize;
}

//********************************
//* ForumView
//********************************

void ForumListControl::enterEvent(QEvent *event)
{
    QListView::enterEvent(event);
    this->verticalScrollBar()->setVisible(true);
}

void ForumListControl::leaveEvent(QEvent *event)
{
    QListView::leaveEvent(event);
    this->verticalScrollBar()->setVisible(false);
}


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

    _listView = new ForumListControl(this);
    _listView->setStyleSheet(strListStyleSheet);
    _listView->setAttribute(Qt::WA_MacShowFocusRect, false);
    _listView->setFont(QFont(_listView->font().family(), TREEFONTSIZE));
    _listView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _listView->verticalScrollBar()->setStyleSheet(strListViewScrollStyle);
    _listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    _listView->setItemDelegate(new ForumViewDelegate);

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
    layout->addWidget(_listView);
//    layout->addWidget(_treeView);

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

    auto oldModel = _listView->model();
    _listView->setModel(model);

//    auto oldModel = _treeView->model();
//    _treeView->setModel(model);
//    _treeView->expandAll();

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
