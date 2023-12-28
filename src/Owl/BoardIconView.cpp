#include <QWidget>
#include <QListView>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QMenu>
#include <QMessageBox>

#include "ZFontIcon/ZFontIcon.h"
#include "ZFontIcon/ZFont_fa5.h"

#include  <Utils/OwlLogger.h>
#include "Data/BoardManager.h"
#include "BoardIconView.h"
#include "Data/ConnectionListModel.h"

constexpr auto LISTICONWIDTH = 164;
constexpr auto LISTICONHEIGHT = 64;

constexpr auto DEFAULT_HOVER = "#222222";
constexpr auto DEFAULT_SELECTED = "black";

constexpr auto INDICATOR_ERROR = "#FF0000";
constexpr auto INDICATOR_LOGGED_IN = "#ADFF2F";
constexpr auto INDICATOR_UNREAD = "#ADFF2F";

#ifdef Q_OS_WINDOWS
    constexpr auto TOP_PADDING = 20;
    constexpr auto ICONDISPLAYWIDTH = 48;
    constexpr auto ICONDISPLAYHEIGHT = 48;
#elif defined(Q_OS_MAC)
    constexpr auto TOP_PADDING = 20;
    constexpr auto ICONDISPLAYWIDTH = 40;
    constexpr auto ICONDISPLAYHEIGHT = 40;
#else
    constexpr auto TOP_PADDING = 5;
    constexpr auto ICONDISPLAYWIDTH = 40;
    constexpr auto ICONDISPLAYHEIGHT = 40;
#endif

using namespace std::literals;
    
namespace owl
{
    
    static const auto BG_COLOR = "#DEDFDF";

// TODO: The "hover" and "select" styles don't really work
// right in `QStyledItemDelegate::paint()` unless the properties
// are defined here, even though they're ignored. Investigate
// this more to find out why
// @NOTE: The `QWidget` in this style sets the color of the area behind
//        the system-buttons (i.e. minimize, close, etc) on Mac
const auto itemStyleSheet = fmt::format(R"(
QListView
{{
    background: {};
    border-style: none;
}}

QListView::item::selected{{ }}
QListView::item::hover{{ }}
)", BG_COLOR);

constexpr std::string_view contextMenuStyle = R"(
QMenu
{
    background-color: #FFFFFF;
}
QMenu::item::selected
{
    background-color: lightgrey;
    color: #000000;
}
)";

QIcon bufferToIcon(const char* buf)
{
    QByteArray buffer(buf);
    QImage image = QImage::fromData(QByteArray::fromBase64(buffer));

    // calculate the scaling factor based on wanting a 32x32 image
    qreal iXScale = static_cast<qreal>(BoardIconModel::ICONSCALEWIDTH) / static_cast<qreal>(image.width());
    qreal iYScale = static_cast<qreal>(BoardIconModel::ICONSCALEHEIGHT) / static_cast<qreal>(image.height());

    // only scale the image if it's not the right size
    if (iXScale > 1 || iXScale < 1 || iYScale > 1 || iYScale < 1)
    {
        QTransform transform;
        transform.scale(iXScale, iYScale);
        image = image.transformed(transform, Qt::SmoothTransformation);
    }

    return QIcon { QPixmap::fromImage(image) };
}


QImage overlayImages(const QImage& baseImage, const QImage& overlaidImg)
{
    // scale our final image to the larger icon size
    QImage finalImage = resizeImage(baseImage, QSize(BoardIconModel::ICONSCALEWIDTH, BoardIconModel::ICONSCALEHEIGHT));

    if (!overlaidImg.isNull())
    {
        constexpr std::double_t widhtScaleFactor = BoardIconModel::ICONSCALEWIDTH * 0.4375;
        constexpr std::double_t heightScaleFactor = BoardIconModel::ICONSCALEHEIGHT * 0.4375;

        // scale the image to be put on top
        QImage scaledOverlayImg { overlaidImg };
        auto iXScale = static_cast<qreal>(widhtScaleFactor) / static_cast<qreal>(scaledOverlayImg.width());
        auto iYScale = static_cast<qreal>(heightScaleFactor) / static_cast<qreal>(scaledOverlayImg.height());
        if (iXScale > 1 || iXScale < 1 || iYScale > 1 || iYScale < 1)
        {
            QTransform transform;
            transform.scale(iXScale, iYScale);
            scaledOverlayImg = scaledOverlayImg.transformed(transform, Qt::SmoothTransformation);
        }

        QPainter p(&finalImage);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.drawImage(finalImage.width() - scaledOverlayImg.width() ,0,scaledOverlayImg);
    }

    return finalImage;
}

//********************************
//* BoardIconViewDelegate
//********************************

void BoardIconViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant decrole = index.data(Qt::DecorationRole);
    if (decrole.type() != QVariant::Icon)
    {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->save();

    // this is our entire drawing area to work with
    QRect iconCellRect { option.rect };
    iconCellRect.setWidth(LISTICONWIDTH);
    iconCellRect.setHeight(LISTICONHEIGHT);

    // this it the `QRect` in which we will draw the board's icon
    QRect iconRect = QRect(0, 0, ICONDISPLAYWIDTH, ICONDISPLAYHEIGHT);
    const std::int32_t hCenterAdjust = (option.rect.width() / 2) - (ICONDISPLAYWIDTH / 2);
    const std::int32_t vCenterAdjust = (option.rect.height() / 2) - (ICONDISPLAYHEIGHT / 2);
    iconRect.moveLeft(iconCellRect.left() + hCenterAdjust);
    iconRect.moveTop(iconCellRect.top() + vCenterAdjust);

    if (index.data(BoardIconModel::ICONTYPE_ROLE).value<IconType>() == IconType::BOARDICON)
    {
        QVariant boardVar = index.data(BoardIconModel::BOARDPTR_ROLE);
        BoardPtr boardData = boardVar.value<BoardWeakPtr>().lock();
        Q_ASSERT(boardData);

        // get a pixmap of the board's stored icon
        const QIcon boardIcon { decrole.value<QIcon>() };
        QPixmap pixmap { boardIcon.pixmap(ICONDISPLAYWIDTH, ICONDISPLAYHEIGHT) };
        QImage boardImg = pixmap.toImage();

        // slightly darken image if it's not selected
        if (boardData->getStatus() != BoardStatus::ONLINE)
        {
            QImage *tmpImage = &boardImg;
            QPainter p(tmpImage);

            QPoint p1, p2;
            p2.setY(tmpImage->height());

            QLinearGradient gradient(p1, p2);

            gradient.setColorAt(0,QColor(0, 0, 0, 65));
            gradient.setColorAt(1,QColor(0, 0, 0, 65));
            p.fillRect(0,0, tmpImage->width(), tmpImage->height(), gradient);

            p.end();
        }

        painter->drawImage(iconRect, boardImg);

        constexpr std::double_t cirlceSize = 6.75;
        constexpr std::double_t circleRadius = cirlceSize / 2;

        QPoint circleCenter = iconRect.center();
        circleCenter.setX(iconRect.right() - static_cast<std::int32_t>(std::floor(circleRadius + 1)));
        circleCenter.setY(iconRect.top() + static_cast<std::int32_t>(std::floor(circleRadius + 2)));

        switch (boardData->getStatus())
        {
            default:
            break;

            case BoardStatus::ONLINE:
            {
                const QColor circleColor = boardData->hasUnread()
                        ? QColor{INDICATOR_UNREAD} : QColor{INDICATOR_LOGGED_IN};

                QPainterPath path;
                path.addEllipse(circleCenter, cirlceSize, cirlceSize);

                QPen pen(QBrush(QColor(Qt::black)), 2);
                painter->setPen(pen);
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->fillPath(path, QColor{circleColor});
                painter->drawPath(path);
            }
            break;

            case BoardStatus::ERR:
            {
                QPainterPath path;
                path.addEllipse(circleCenter, cirlceSize, cirlceSize);

                QPen pen(QBrush(QColor(Qt::black)), 2);
                painter->setPen(pen);
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->fillPath(path, QColor{INDICATOR_ERROR});
                painter->drawPath(path);
            }
            break;
        }
    }
    else
    {
        // Q_ASSERT(index.data(BoardIconModel::ICONTYPE_ROLE).value<IconType>() == IconType::ADDICON);

        // get a pixmap of the board's stored icon
        const QIcon icon { decrole.value<QIcon>() };
        const QPixmap pixmap { icon.pixmap(ICONDISPLAYWIDTH, ICONDISPLAYHEIGHT) };
        painter->drawPixmap(iconRect, pixmap);
    }

    if (option.state & QStyle::State_Selected)
    {
        QPen pen2{QBrush{QColor{DEFAULT_SELECTED}}, 10.00};
        painter->setPen(pen2);
        painter->setRenderHint(QPainter::Antialiasing, true);

        iconRect.setWidth(6);
        iconRect.setHeight(6);
        iconRect.adjust(-22, 18, -22, 18);
        painter->drawRoundedRect(iconRect, 10.0, 10.0);
    }
    else if (option.state & QStyle::State_MouseOver)
    {
        QPen pen(QBrush(QColor(DEFAULT_HOVER)), 10.00);
        painter->setPen(pen);
        painter->setRenderHint(QPainter::Antialiasing, true);

        iconRect.setWidth(10);
        iconRect.setHeight(25);
        iconRect.adjust(-25, 7, -25, 7);
        painter->drawRoundedRect(iconRect, 5.0, 5.0);
    }

    painter->restore();
}

QSize BoardIconViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

//********************************
//* BoardIconView
//********************************

BoardIconView::BoardIconView(QWidget* parent /* = 0*/)
    : QWidget(parent),
      _logger { owl::initializeLogger("BoardIconView") }
{
    parent->setStyleSheet((fmt::format("QWidget{{ background-color: {}; }}", BG_COLOR)).data());
    createListView();

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addSpacing(TOP_PADDING);
    layout->addWidget(_listView);
    setLayout(layout);
}

void BoardIconView::createListView()
{
    _listView = new QListView(this);
    _listView->setViewMode(QListView::IconMode);
    _listView->setFlow(QListView::TopToBottom);
    _listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    _listView->setUniformItemSizes(true);
    _listView->setWrapping(false);
    _listView->setAttribute(Qt::WA_MacShowFocusRect, false);
    _listView->setMovement(QListView::Movement::Static);
    _listView->setStyleSheet(QString::fromLatin1(itemStyleSheet.data()));
    _listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _listView->setContextMenuPolicy(Qt::CustomContextMenu);
    _listView->setIconSize(QSize(LISTICONWIDTH, LISTICONHEIGHT));
    _listView->setItemDelegate(new BoardIconViewDelegate);
}

void BoardIconView::initListView()
{
    _listView->setModel(_connectionModel);
    // _listView->setModel(new owl::BoardIconModel(this));

    QObject::connect(_listView, &QWidget::customContextMenuRequested, this,
        [this](const QPoint &pos)
        {
            this->doContextMenu(pos);
        });

    QObject::connect(_listView, &QAbstractItemView::clicked, this,
        [this](const QModelIndex& index)
        {
            const auto type = index.data(owl::ConnectionRoles::TYPE).value<owl::ConnectionType>();
            switch (type)
            {
                default:
                {
                    const auto uuid = index.data(owl::ConnectionRoles::UUID).toString().toStdString();
                    Q_EMIT onConnectionClicked(uuid);
                }
                break;

                case owl::ConnectionType::CHAT_BUTTON:
                    Q_EMIT onChatButtonClicked();
                break;

                case owl::ConnectionType::NEW_CONNECTION_BUTTON:
                    Q_EMIT onNewConnectionButtonClicked();
                break;
            }


            // if (index.data(BoardIconModel::ICONTYPE_ROLE).value<IconType>() == IconType::BOARDICON)
            // {
            //     QVariant boardVar = index.data(BoardIconModel::BOARDPTR_ROLE);
            //     owl::BoardWeakPtr weakBoard = boardVar.value<BoardWeakPtr>();
            //     if (auto board = weakBoard.lock(); board && board.get() != _rawBoardPtr)
            //     {
            //         _rawBoardPtr = board.get();
            //         Q_EMIT onBoardClicked(weakBoard);
            //     }
            // }
            // else if (index.data(BoardIconModel::ICONTYPE_ROLE).value<IconType>() == IconType::ADDICON)
            // {
            //     Q_EMIT onAddNewBoard();
            // }
            // else if (index.data(BoardIconModel::ICONTYPE_ROLE).value<IconType>() == IconType::WEBICON)
            // {
            //     _rawBoardPtr = nullptr;
            //     Q_EMIT onAddNewWebBrowser();
            // }
        });

    QObject::connect(_listView, &QAbstractItemView::doubleClicked, this,
        [this](const QModelIndex &index)
        {
            if (index.data(BoardIconModel::ICONTYPE_ROLE).value<IconType>() == IconType::BOARDICON)
            {
                if (index.isValid())
                {
                    QVariant boardVar = index.data(BoardIconModel::BOARDPTR_ROLE);
                    Q_ASSERT(!boardVar.isNull() && boardVar.isValid());
                    Q_EMIT onBoardDoubleClicked(boardVar.value<BoardWeakPtr>());
                }
            }
    });
}

void BoardIconView::doContextMenu(const QPoint &pos)
{
    const auto index = _listView->indexAt(pos);
    if (QVariant boardVar = index.data(BoardIconModel::BOARDPTR_ROLE);
        boardVar.canConvert<BoardWeakPtr>())
    {
        BoardPtr boardPtr = boardVar.value<BoardWeakPtr>().lock();
        QMenu* menu = new QMenu(this);
        menu->setStyleSheet(contextMenuStyle.data());

        if (boardPtr->getStatus() == BoardStatus::OFFLINE)
        {
            QAction* action = menu->addAction(tr("Connect"));
            action->setToolTip(tr("Connect"));
            QObject::connect(action, &QAction::triggered, this,
                [this, boardVar]()
                {
                    Q_EMIT onConnectBoard(boardVar.value<BoardWeakPtr>());
                });

            menu->addSeparator();
        }
        else if (boardPtr->getStatus() == BoardStatus::ONLINE)
        {
            QAction* action = menu->addAction(tr("Mark All Forums Read"));
            action->setToolTip(tr("Mark All Forums Read"));
#ifdef Q_OS_MACX
            action->setIconVisibleInMenu(false);
#endif

            connect(action, &QAction::triggered, this,
                [this, boardVar]()
                {
                    Q_EMIT onMarkBoardRead(boardVar.value<BoardWeakPtr>());
                });
        }

        {
            QAction* action = menu->addAction(tr("Copy Board Address"));
            action->setToolTip(tr("Copy Board Address"));
            connect(action, &QAction::triggered, this,
                [this, boardVar]()
                {
                    Q_EMIT onCopyBoardAddress(boardVar.value<BoardWeakPtr>());
                });
        }

        {
            QAction* action = menu->addAction(tr("Open in Browser"));
            action->setToolTip(tr("Open in Browser"));
#ifdef Q_OS_MACX
            action->setIconVisibleInMenu(false);
#endif

            connect(action, &QAction::triggered, this,
                [this, boardVar]()
                {
                    Q_EMIT onOpenBoardInBrowser(boardVar.value<BoardWeakPtr>());
                });
        }

        menu->addSeparator();

        {
            QAction* action = menu->addAction(tr("Settings"));
#ifdef Q_OS_MACX
            action->setIconVisibleInMenu(false);
#endif

            QObject::connect(action, &QAction::triggered, this,
                [this, boardVar]()
                {
                    Q_EMIT onEditBoard(boardVar.value<BoardWeakPtr>());
                });
        }

        {
            QAction* action = menu->addAction(tr("Delete"));
            QObject::connect(action, &QAction::triggered, this,
                [this, boardVar]()
                {
                    this->requestBoardDelete(boardVar.value<BoardWeakPtr>());
                });
        }

        menu->exec(this->mapToGlobal(pos));
    }
}

void BoardIconView::requestBoardDelete(BoardWeakPtr boardWeak)
{
    BoardPtr board = boardWeak.lock();
    if (board)
    {
        const QString strMsg = QString(tr("Are you sure you want to delete the board \"%1\"?\n\n"
            "This will permanently remove all data associated with this board.")).arg(board->getName());

        QMessageBox messageBox(
            QMessageBox::Question,
            tr("Delete Message Board"),
            strMsg,
            QMessageBox::Yes | QMessageBox::No,
            this,
            Qt::Drawer);

        messageBox.setWindowModality(Qt::WindowModal);

        if (messageBox.exec() == QMessageBox::Yes)
        {
            Q_EMIT onDeleteBoard(boardWeak);
        }
    }
}

} // namespace
