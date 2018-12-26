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
    #define BOARDNAMEFONT       14
    #define USERNAMEFONT        11
    #define TREEFONTSIZE        12
#endif

static const char* strListStyleSheet = R"(
QListView
{
    background: #594157;
    border-style: none;
}

QListView::item::selected
{
    background: #587B7F;
}

QListView::item::hover
{
    background: #222222;
}
)";

static const char* strListViewScrollStyle = R"(
QScrollBar:vertical {
    border: 0px;
    background: #594157;
    width: 5px;
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
    QStyleOptionViewItem styledOption{option};

    owl::Forum* item = static_cast<owl::Forum*>(index.internalPointer());
    if (item->getForumType() == owl::Forum::ForumType::CATEGORY)
    {
        painter->save();

        if (option.state & QStyle::State_MouseOver)
        {
            painter->setPen(QPen(Qt::white));
        }
        else
        {
            painter->setPen(QColor("#f2f2f2"));
        }

        QRect textRect { option.rect };
        textRect.adjust(5, 1, 0, -7);

        QFont font{ option.font };
        font.setBold(true);
        QFontMetrics metrics(font);
        QString elidedText = metrics.elidedText(item->getName(), Qt::ElideRight, textRect.width());

        painter->setFont(font);
        painter->drawText(textRect, Qt::AlignBottom, elidedText);
        painter->restore();
    }
    else
    {
        painter->save();

        // draw the background first
        QColor bgColor("#594157");
        if (option.state & QStyle::State_Selected)
        {
            bgColor = QColor("#587B7F");
        }
        else if (option.state & QStyle::State_MouseOver)
        {
            bgColor = QColor("#25383C");
        }
        painter->fillRect(option.rect, bgColor);

        QImage image;
        owl::Forum* item = static_cast<owl::Forum*>(index.internalPointer());

        if (item->getForumType() == owl::Forum::ForumType::FORUM)
        {
            image = QImage(":/icons/forum.png");
        }
        else if (item->getForumType() == owl::Forum::ForumType::LINK)
        {
            image = QImage(":/icons/link.png");
        }

        // center the image vertically
        int yAdjust = static_cast<int>((option.rect.height() - image.size().height()) / 2);

        // draw the image centered
        QRect workingRect{ option.rect };
        workingRect.adjust(5, yAdjust, 0, 0);
        painter->drawImage(QPoint(workingRect.x(), workingRect.y()), image);

        // now adjust for the text
        workingRect = option.rect;
        QFontMetrics metrics(option.font);
        yAdjust = static_cast<int>((option.rect.height() - metrics.height()) / 2);
        workingRect.adjust(25, yAdjust, -1, 0);
        QString elidedText = metrics.elidedText(item->getName(), Qt::ElideRight, workingRect.width());
        painter->setPen(QColor("#f2f2f2"));
        painter->drawText(workingRect, elidedText);

        painter->restore();
    }
}

QSize ForumViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize retsize { option.rect.size() };
    retsize.setHeight(30);

    owl::Forum* item = static_cast<owl::Forum*>(index.internalPointer());
    if (item->getForumType() == owl::Forum::ForumType::CATEGORY
            && index.model()->hasIndex(index.row() - 1, index.column()))
    {
        auto prevIdx = index.model()->index(index.row() - 1, index.column());
        owl::Forum* previtem = static_cast<owl::Forum*>(prevIdx.internalPointer());
        if (previtem->getForumType() == owl::Forum::ForumType::FORUM)
        {
            retsize.setHeight(50);
        }
    }

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

void ForumView::initListView()
{
    _listView = new ForumListControl(this);
    _listView->setStyleSheet(strListStyleSheet);
    _listView->setAttribute(Qt::WA_MacShowFocusRect, false);
    _listView->setFont(QFont(_listView->font().family(), TREEFONTSIZE));
    _listView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _listView->verticalScrollBar()->setStyleSheet(strListViewScrollStyle);
    _listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _listView->setItemDelegate(new ForumViewDelegate);

    QObject::connect(_listView, &QAbstractItemView::clicked,
        [this](const QModelIndex& index)
        {
            owl::Forum* forum = static_cast<owl::Forum*>(index.internalPointer());
            if (forum->getForumType() == owl::Forum::ForumType::FORUM)
            {
                owl::ForumPtr forumPtr = index.data(SHAREDPTR_ROLE).value<owl::ForumPtr>();
                Q_ASSERT(forumPtr);
                Q_EMIT onForumClicked(forumPtr);
            }
        });
}

ForumView::ForumView(QWidget* parent /* = 0*/)
    : QWidget(parent),
      _logger { owl::initializeLogger("ForumView") }
{
    // down below we set the `layout` margin to 5, which means that the
    // parent's color gets drawn in that margin area, so we have to set
    // the parent's color to match
    parent->setStyleSheet("QWidget { background-color: #594157; }");
    setStyleSheet("QWidget { background-color: #594157; border: none; }");

    initListView();

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
    layout->setMargin(0);

    layout->addWidget(_boardLabel);
    layout->addLayout(userLayout);
    layout->addItem(new QSpacerItem(0,15));
    layout->addWidget(_listView);

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

    QFontMetrics metrics(_boardLabel->font());
    QString elidedText = metrics.elidedText(currentBoard->getName(), Qt::ElideRight, _boardLabel->rect().width());
    _boardLabel->setText(elidedText);

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
