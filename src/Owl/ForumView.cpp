#include <QVBoxLayout>
#include <QLabel>
#include <QScrollBar>

#include <fmt/compile.h>

#include "ZFontIcon/ZFontIcon.h"
#include "ZFontIcon/ZFont_fa5.h"
#include "ZFontIcon/ZFont_fa4.h"

#include  <Utils/OwlLogger.h>

#include "Data/Board.h"
#include "Data/ForumTreeModel.h"

#include "ForumView.h"

#if defined(Q_OS_WIN)
    #define BOARDNAMEFONT       12
    #define USERNAMEFONT        10
    #define TREEFONTSIZE        10
    #define TREEITEMHEIGHT      30
    #define TREECATHEIGHT       50
    #define TOP_PADDING         10
    #define LEFT_PADDING        2
#elif defined(Q_OS_MAC)
    #define BOARDNAMEFONT       20
    #define USERNAMEFONT        15
    #define TREEFONTSIZE        14
    #define TREEITEMHEIGHT      38
    #define TREECATHEIGHT       48
    #define TOP_PADDING         23
    #define LEFT_PADDING        6
#else
    #define BOARDNAMEFONT       14
    #define USERNAMEFONT        11
    #define TREEFONTSIZE        12
    #define TREEITEMHEIGHT      30
    #define TREECATHEIGHT       50
    #define TOP_PADDING         20
    #define LEFT_PADDING        2
#endif

using namespace std::literals;

static const auto BG_COLOR =            "#F3F3F4"sv;
static const auto HEADER_COLOR =        "#5c5e66"sv;
static const auto USERNAME_COLOR =      "#5c5e66"sv;
static const auto SUB_COLOR =           "#6d6f77"sv;
static const auto FORUM_COLOR =         "#5c5e66"sv;
static const auto HOVER_COLOR=          "#e0e1e5"sv;
static const auto SELECTED_COLOR=       "#d7d9dc"sv;

const auto strListStyleSheet = fmt::format(R"x(
QListView
{{
    background: {};
    border-style: none;
}}

QListView::item::selected
{{
    background: yellow;
}}

QListView::item::hover
{{
    background: blue;
}}
)x", BG_COLOR);

const auto strListViewScrollStyle = fmt::format(R"(
QScrollBar:vertical
{{
    border: 0px;
    background: {};
    width: 5px;
    margin: 0px 0px 0px 0px;
}}

QScrollBar::handle:vertical
{{
    background: darkgrey;
}}

QScrollBar::add-line:vertical
{{
    height: 0px;
    subcontrol-position: bottom;
    subcontrol-origin: margin;
}}

QScrollBar::sub-line:vertical
{{
    height: 0 px;
    subcontrol-position: top;
    subcontrol-origin: margin;
}}
)", BG_COLOR);

namespace owl
{

//********************************
//* ForumViewDelegate
//********************************

ForumViewDelegate::ForumViewDelegate()
{
    _forumUnreadIcon = QIcon(ZFontIcon::icon(Fa4::FAMILY, Fa4::fa_commenting, QColor{120,120,120}, 0.85));
    _forumReadIcon = QIcon(ZFontIcon::icon(Fa4::FAMILY, Fa4::fa_commenting_o));
}

void ForumViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem styledOption{option};

    owl::Forum* item = static_cast<owl::Forum*>(index.internalPointer());
    if (item->getForumType() == owl::Forum::ForumType::CATEGORY)
    {
        painter->save();
        
        QFont font{ option.font };
        font.setBold(true);
        font.setCapitalization(QFont::Capitalization::AllUppercase);
        font.setPointSize(font.pointSize() * 0.925f);
        painter->setPen(QPen(QColor(SUB_COLOR.data())));

        QRect textRect { option.rect };
        textRect.adjust(5, 1, 0, -7);

        QFontMetrics metrics(font);
        QString elidedText = metrics.elidedText(item->getName(), Qt::ElideRight, textRect.width());

        painter->setFont(font);
        painter->drawText(textRect, Qt::AlignBottom, elidedText);
        painter->restore();
    }
    else
    {
        static int x = 0;
        x--;
        painter->save();

        // draw the background first
        QColor bgColor(BG_COLOR.data());
        if (option.state & QStyle::State_Selected)
        {
            bgColor = QColor(SELECTED_COLOR.data());
        }
        else if (option.state & QStyle::State_MouseOver)
        {
            bgColor = QColor(HOVER_COLOR.data());
        }
        painter->fillRect(option.rect, bgColor);

        owl::Forum* item = static_cast<owl::Forum*>(index.internalPointer());
        QImage image;

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
//        painter->drawImage(QPoint(workingRect.x(), workingRect.y()), image);

        const QIcon* board_icon;
        if (item->hasUnread())
        {
            board_icon = &_forumUnreadIcon;
        }
        else
        {
            board_icon = &_forumReadIcon;
        }

        const QPixmap board_map = board_icon->pixmap(QSize{24, 24});
        painter->drawPixmap(QPoint(workingRect.x() - 5, workingRect.y() - 7), board_map);

//        QRect iconRect { workingRect };
//        iconRect.adjust(0,0, -64, 0);


////        const auto board_icon_size = workingRect.size();
//        const QIcon board_icon = QIcon(ZFontIcon::icon(Fa5::FAMILY, Fa5::fa_plus_circle));
////        qDebug() << "QIcon: " << board_icon
//        qDebug() << "Rect: " << workingRect;
//        board_icon.paint(painter, workingRect, Qt::AlignLeft);
////        painter->drawPixmap(QPoint(workingRect.x(), workingRect.y()), board_icon);
        
        if (item->hasUnread())
        {
            QFont newfont { option.font };
            newfont.setBold(true);
            painter->setFont(newfont);
            painter->setPen(QColor{"black"});
        }
        else
        {
            painter->setFont(option.font);
            painter->setPen(QColor(FORUM_COLOR.data()));
        }

        // now adjust for the text
        workingRect = option.rect;
        QFontMetrics metrics(painter->font());
        yAdjust = static_cast<int>((option.rect.height() - metrics.height()) / 2);
        workingRect.adjust(25, yAdjust, -1, 0);
        QString elidedText = metrics.elidedText(item->getName(), Qt::ElideRight, workingRect.width());
        painter->drawText(workingRect, elidedText);

        painter->restore();
    }
}

QSize ForumViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QSize retsize { option.rect.size() };
    retsize.setHeight(TREEITEMHEIGHT);

    owl::Forum* item = static_cast<owl::Forum*>(index.internalPointer());
    if (item->getForumType() == owl::Forum::ForumType::CATEGORY
            && index.model()->hasIndex(index.row() - 1, index.column()))
    {
        auto prevIdx = index.model()->index(index.row() - 1, index.column());
        owl::Forum* previtem = static_cast<owl::Forum*>(prevIdx.internalPointer());
        if (previtem->getForumType() == owl::Forum::ForumType::FORUM)
        {
            retsize.setHeight(TREECATHEIGHT);
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
    _listView->setStyleSheet(strListStyleSheet.data());
    _listView->setAttribute(Qt::WA_MacShowFocusRect, false);
    _listView->setFont(QFont(_listView->font().family(), TREEFONTSIZE));
    _listView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _listView->verticalScrollBar()->setStyleSheet(strListViewScrollStyle.data());
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
    parent->setStyleSheet((fmt::format("QWidget{{ background-color: {}; }}", BG_COLOR)).data());
    setStyleSheet((fmt::format("QWidget{{ background-color: {}; border: none; }}", BG_COLOR)).data());

    initListView();

    _boardLabel = new QLabel(this);
    QFont font;
    font.setPointSize(BOARDNAMEFONT);
    font.setBold(true);
    font.setWeight(75);
    _boardLabel->setFont(font);
    _boardLabel->setStyleSheet(fmt::format("QLabel{{ color : {}; }}", HEADER_COLOR).data());

    QHBoxLayout* boardNameLayout = new QHBoxLayout();
    boardNameLayout->addSpacing(0);
    boardNameLayout->addWidget(_boardLabel);

    QHBoxLayout* userLayout = new QHBoxLayout();
    _userLabel = new QLabel(this);
    _userLabel->setMaximumHeight(64);
    font.setPointSize(USERNAMEFONT);
    font.setBold(false);
    _userLabel->setFont(font);
    _userLabel->setStyleSheet(fmt::format("QLabel{{ color : {}; }}", USERNAME_COLOR).data());

    _userImgLabel = new QLabel(this);
    _userImgLabel->setMaximumHeight(64);
    _userImgLabel->setMaximumWidth(16);

    userLayout->addWidget(_userImgLabel);
    userLayout->addWidget(_userLabel);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addSpacing(TOP_PADDING);
    layout->addLayout(boardNameLayout);
    layout->addLayout(userLayout);
    layout->addItem(new QSpacerItem(0,15));
    layout->addWidget(_listView);

    QHBoxLayout* rootLayout = new QHBoxLayout();
    rootLayout->setSpacing(0);
    rootLayout->setMargin(0);
    rootLayout->addSpacing(LEFT_PADDING);
    rootLayout->addLayout(layout);

    setLayout(rootLayout);
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

    ForumTreeModel* model{ nullptr };
    if (_rootCache.contains(currentBoard->hash()))
    {
        auto& [expiry, cacheModel] = *(_rootCache.object(currentBoard->hash()));
        if (QDateTime::currentDateTime() < expiry)
        {
            model = cacheModel;
        }
    }
    
    if (!model)
    {
        _rootCache.remove(currentBoard->hash());
        ForumPtr root = currentBoard->getRootStructure(false);
        model = new ForumTreeModel{ root };

        // only add the model to the cache if there's something in it
        if (root->getForums().size() > 0 || root->getThreads().size() > 0)
        {
            QDateTime expiry{ QDateTime::currentDateTime() };
            expiry = expiry.addSecs(60 * 10);
            CacheEntry* entry = new CacheEntry(expiry, model);
            _rootCache.insert(currentBoard->hash(), entry);
        }
    }

    Q_ASSERT(model);
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

    Q_EMIT onForumListLoaded();
}

} // namespace
