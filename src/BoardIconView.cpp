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

#include  <Utils/OwlLogger.h>

#include "Data/BoardManager.h"
#include "Utils/Exception.h"

#include "DefaultStyle.h"
#include "BoardIconView.h"

#define ICONSCALEWIDTH       128
#define ICONSCALEHEIGHT      128

#define ICONDISPLAYWIDTH      38
#define ICONDISPLAYHEIGHT     38

#define LISTICONWIDTH         70
#define LISTICONHEIGHT        64

namespace owl
{

// TODO: The "hover" and "select" styles don't really work
// right in `QStyledItemDelegate::paint()` unless the properties
// are defined here, even though they're ignored. Investigate
// this more to find out why
static const char* itemStyleSheet = R"(
QListView
{
    background: #181F26;
    border-style: none;
}

QListView::item::selected{}
QListView::item::hover{}
)";

QIcon bufferToIcon(const char* buf)
{
    QByteArray buffer(buf);
    QImage image = QImage::fromData(QByteArray::fromBase64(buffer));

    // calculate the scaling factor based on wanting a 32x32 image
    qreal iXScale = static_cast<qreal>(ICONSCALEWIDTH) / static_cast<qreal>(image.width());
    qreal iYScale = static_cast<qreal>(ICONSCALEHEIGHT) / static_cast<qreal>(image.height());

    // only scale the image if it's not the right size
    if (iXScale > 1 || iXScale < 1 || iYScale > 1 || iYScale < 1)
    {
        QTransform transform;
        transform.scale(iXScale, iYScale);
        image = image.transformed(transform, Qt::SmoothTransformation);
    }

    return QIcon { QPixmap::fromImage(image) };
}

QImage resizeImage(const QImage& original, const QSize& size)
{
    QImage finalImage { original };
    qreal iXScale = static_cast<qreal>(size.width()) / static_cast<qreal>(finalImage.width());
    qreal iYScale = static_cast<qreal>(size.height()) / static_cast<qreal>(finalImage.height());
    if (iXScale > 1 || iXScale < 1 || iYScale > 1 || iYScale < 1)
    {
        QTransform transform;
        transform.scale(iXScale, iYScale);
        finalImage = finalImage.transformed(transform, Qt::SmoothTransformation);
    }

    return finalImage;
}

QImage overlayImages(const QImage& baseImage, const QImage& overlaidImg)
{
    // scale our final image to the larger icon size
    QImage finalImage = resizeImage(baseImage, QSize(ICONSCALEWIDTH, ICONSCALEHEIGHT));

    if (!overlaidImg.isNull())
    {
        constexpr std::double_t widhtScaleFactor = ICONSCALEWIDTH * 0.4375;
        constexpr std::double_t heightScaleFactor = ICONSCALEHEIGHT * 0.4375;

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

    if (index.data(ICONTYPE_ROLE).value<IconType>() == IconType::BOARDICON)
    {
        QVariant boardVar = index.data(BOARDPTR_ROLE);
        BoardPtr boardData = boardVar.value<BoardWeakPtr>().lock();
        Q_ASSERT(boardData);

        // get a pixmap of the board's stored icon
        const QIcon boardIcon { decrole.value<QIcon>() };
        QPixmap pixmap { boardIcon.pixmap(ICONDISPLAYWIDTH, ICONDISPLAYHEIGHT) };

        switch (boardData->getStatus())
        {
            case BoardStatus::ONLINE:
            {
                painter->drawPixmap(iconRect, pixmap);
                break;
            }
            case BoardStatus::OFFLINE:
            {
                QImage boardImg = pixmap.toImage().convertToFormat(QImage::Format_Grayscale8);
                painter->drawImage(iconRect, boardImg);
                break;
            }
            case BoardStatus::ERR:
            {
                QImage boardImg = pixmap.toImage().convertToFormat(QImage::Format_Grayscale8);
                static const QImage errorImage { ":/icons/error_32.png" };

                boardImg = overlayImages(boardImg, errorImage);
                painter->drawImage(iconRect, boardImg);
                break;
            }
        }

        if (option.state & QStyle::State_Selected)
        {
            auto padding = (iconCellRect.height() - iconRect.height()) / 2;
            padding += 4;

            QRect selectRect { iconCellRect };
            selectRect.moveRight(selectRect.left() + 1);
            selectRect.adjust(0, padding + 5, 0, -padding);

            QPen pen(QBrush(QColor(DEFAULT_SELECTED)), 6);
            painter->setPen(pen);
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->drawRoundRect(selectRect);
        }
    }
    else
    {
        Q_ASSERT(index.data(ICONTYPE_ROLE).value<IconType>() == IconType::ADDICON);

        // get a pixmap of the board's stored icon
        const QIcon icon { decrole.value<QIcon>() };
        const QPixmap pixmap { icon.pixmap(ICONDISPLAYWIDTH, ICONDISPLAYHEIGHT) };
        painter->drawPixmap(iconRect, pixmap);
    }

    if (option.state & QStyle::State_MouseOver)
    {
        QPen pen(QBrush(QColor(DEFAULT_HOVER)), 4);
        painter->setPen(pen);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawRoundRect(iconRect);
    }

    painter->restore();
}

QSize BoardIconViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

//********************************
//* BoardIconListView
//********************************

void BoardIconListView::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    // do we need to call the base method?
    QListView::currentChanged(current, previous);
    Q_EMIT onIndexChanged(current);
}

//********************************
//* BoardIconModel
//********************************

BoardIconModel::BoardIconModel(QObject *parent)
    : QAbstractListModel(parent),
      _boardManager(owl::BoardManager::instance())
{}

QModelIndex BoardIconModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) return QModelIndex{};

    Q_ASSERT(!parent.isValid());
    Q_ASSERT(column == 0);


    std::size_t trow = static_cast<std::size_t>(row);
    if (trow < _boardManager->getBoardCount())
    {
        return createIndex(row, column, _boardManager->boardByIndex(trow).get());
    }

    return createIndex(row, column);
}

int BoardIconModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0) return 0;
    return static_cast<int>(_boardManager->getBoardCount() + 1);
}

QVariant BoardIconModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant{};

    if (index.row() < static_cast<int>(_boardManager->getBoardCount()))
    {
        switch (role)
        {
            case Qt::DecorationRole:
            {
                owl::Board* board = static_cast<owl::Board*>(index.internalPointer());
                Q_ASSERT(board);

                QByteArray buffer(board->getFavIcon().toLatin1());

                QImage image = QImage::fromData(QByteArray::fromBase64(buffer));
                image = resizeImage(image, QSize(ICONSCALEWIDTH, ICONSCALEHEIGHT));
                return QIcon { QPixmap::fromImage(image) };
            }

            case ICONTYPE_ROLE:
                return QVariant::fromValue(IconType::BOARDICON);

            case BOARDPTR_ROLE:
            {
                std::size_t trow = static_cast<std::size_t>(index.row());
                std::weak_ptr<owl::Board> retval { _boardManager->boardByIndex(trow) };
                return QVariant::fromValue(retval);
            }
        }
    }
    else
    {
        switch (role)
        {
            case Qt::DecorationRole:
                return QVariant { QIcon("://icons/add-board-512.png") };

            case ICONTYPE_ROLE:
                return QVariant::fromValue(IconType::ADDICON);
        }
    }

    return QVariant{};
}

//********************************
//* BoardIconView
//********************************

BoardIconView::BoardIconView(QWidget* parent /* = 0*/)
    : QWidget(parent),
      _logger { owl::initializeLogger("BoardIconView") }
{
    initListView();

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget(_listView);

    setLayout(layout);
}

void BoardIconView::initListView()
{
    _listView = new BoardIconListView(this);
    _listView->setViewMode(QListView::IconMode);
    _listView->setFlow(QListView::TopToBottom);
    _listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    _listView->setUniformItemSizes(true);
    _listView->setWrapping(false);
    _listView->setAttribute(Qt::WA_MacShowFocusRect, false);
    _listView->setMovement(QListView::Movement::Static);
    _listView->setStyleSheet(QString::fromLatin1(itemStyleSheet));
    _listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _listView->setContextMenuPolicy(Qt::CustomContextMenu);
    _listView->setIconSize(QSize(LISTICONWIDTH, LISTICONHEIGHT));

    _listView->setItemDelegate(new BoardIconViewDelegate);
    _listView->setModel(new owl::BoardIconModel(this));

    QObject::connect(_listView, &QWidget::customContextMenuRequested,
        [this](const QPoint &pos) { this->doContextMenu(pos); });

    QObject::connect(_listView, &BoardIconListView::onIndexChanged,
        [this](const QModelIndex &index)
        {
            if (index.data(ICONTYPE_ROLE).value<IconType>() == IconType::BOARDICON)
            {
                if (index.isValid())
                {
                    QVariant boardVar = index.data(BOARDPTR_ROLE);
                    Q_ASSERT(!boardVar.isNull() && boardVar.isValid());
                    Q_EMIT onBoardClicked(boardVar.value<BoardWeakPtr>());
                }
            }
            else
            {
                Q_ASSERT(index.data(ICONTYPE_ROLE).value<IconType>() == IconType::ADDICON);
                Q_EMIT onAddNewBoard();
            }
        });

    QObject::connect(_listView, &QAbstractItemView::doubleClicked,
        [this](const QModelIndex &index)
        {
            if (index.data(ICONTYPE_ROLE).value<IconType>() == IconType::BOARDICON)
            {
                if (index.isValid())
                {
                    QVariant boardVar = index.data(BOARDPTR_ROLE);
                    Q_ASSERT(!boardVar.isNull() && boardVar.isValid());
                    Q_EMIT onBoardDoubleClicked(boardVar.value<BoardWeakPtr>());
                }
            }
    });
}

void BoardIconView::doContextMenu(const QPoint &pos)
{
    const auto index = _listView->indexAt(pos);
    if (QVariant boardVar = index.data(BOARDPTR_ROLE);
        boardVar.canConvert<BoardWeakPtr>())
    {
        BoardPtr boardPtr = boardVar.value<BoardWeakPtr>().lock();
        QMenu* menu = new QMenu(this);

        if (boardPtr->getStatus() == BoardStatus::OFFLINE)
        {
            QAction* action = menu->addAction(tr("Connect"));
            action->setToolTip(tr("Connect"));
            QObject::connect(action, &QAction::triggered,
                [this, boardVar]()
                {
                    onConnectBoard(boardVar.value<BoardWeakPtr>());
                });

            menu->addSeparator();
        }

        if (boardPtr->getStatus() == BoardStatus::ONLINE)
        {
            QAction* action = menu->addAction(QIcon(":/icons/markforumread.png"), tr("Mark All Forums Read"));
            action->setToolTip(tr("Mark All Forums Read"));
#ifdef Q_OS_MACX
            action->setIconVisibleInMenu(false);
#endif

            connect(action, &QAction::triggered,
                [this, boardVar]()
                {
                    onMarkBoardRead(boardVar.value<BoardWeakPtr>());
                });
        }

        menu->addSeparator();

        {
            QAction* action = menu->addAction(tr("Copy Board Address"));
            action->setToolTip(tr("Copy Board Address"));
            connect(action, &QAction::triggered,
                [this, boardVar]()
                {
                    onCopyBoardAddress(boardVar.value<BoardWeakPtr>());
                });
        }

        {
            QAction* action = menu->addAction(QIcon(":/icons/link.png"), tr("Open in Browser"));
            action->setToolTip(tr("Open in Browser"));
#ifdef Q_OS_MACX
            action->setIconVisibleInMenu(false);
#endif

            connect(action, &QAction::triggered,
                [this, boardVar]()
                {
                    onOpenBoardInBrowser(boardVar.value<BoardWeakPtr>());
                });
        }

        menu->addSeparator();

        {
            QAction* action = menu->addAction(QIcon(":/icons/settings.png"), tr("Settings"));
#ifdef Q_OS_MACX
            action->setIconVisibleInMenu(false);
#endif

            QObject::connect(action, &QAction::triggered,
                [this, boardVar]()
                {
                    onEditBoard(boardVar.value<BoardWeakPtr>());
                });
        }

        {
            QAction* action = menu->addAction(QIcon(":/icons/delete.png"), tr("Delete"));
            QObject::connect(action, &QAction::triggered,
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
            onDeleteBoard(boardWeak);
        }
    }
}

} // namespace
