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

#include  <Utils/OwlLogger.h>

#include "Data/BoardManager.h"
#include "Utils/Exception.h"
#include "BoardIconView.h"

#define ICONSCALEWIDTH       128
#define ICONSCALEHEIGHT      128

#define LISTICONWIDTH         70
#define LISTICONHEIGHT        64

#define ICONDISPLAYWIDTH      38
#define ICONDISPLAYHEIGHT     38

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

QListView::item::selected
{
    border-color: #93C0A4;
    border-style: outset;
    border-width: 2px;
    border-radius: 5px;
}

QListView::item::hover
{
    border-color: #606060;
    border-style: outset;
    border-width: 3px;
    border-radius: 5px;
}
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

QImage overlayImages(const QImage& baseImage, const QImage& overlaidImg)
{
    // scale our final image to the larger icon size
    QImage finalImage { baseImage };
    qreal iXScale = static_cast<qreal>(ICONSCALEWIDTH) / static_cast<qreal>(finalImage.width());
    qreal iYScale = static_cast<qreal>(ICONSCALEHEIGHT) / static_cast<qreal>(finalImage.height());
    if (iXScale > 1 || iXScale < 1 || iYScale > 1 || iYScale < 1)
    {
        QTransform transform;
        transform.scale(iXScale, iYScale);
        finalImage = finalImage.transformed(transform, Qt::SmoothTransformation);
    }

    if (!overlaidImg.isNull())
    {
        // scale the image to be put on top
        QImage scaledOverlayImg { overlaidImg };
        iXScale = static_cast<qreal>(56) / static_cast<qreal>(scaledOverlayImg.width());
        iYScale = static_cast<qreal>(56) / static_cast<qreal>(scaledOverlayImg.height());
        if (iXScale > 1 || iXScale < 1 || iYScale > 1 || iYScale < 1)
        {
            QTransform transform;
            transform.scale(iXScale, iYScale);
            scaledOverlayImg = scaledOverlayImg.transformed(transform, Qt::SmoothTransformation);
        }

        QPainter p(&finalImage);
        p.setCompositionMode(QPainter::CompositionMode_Source);
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

    // get the board data
    std::shared_ptr<Board> boardData;
    if (QVariant boardVar = index.data(BOARDPTR_ROLE);
        boardVar.canConvert<BoardWeakPtr>())
    {
        auto weakPtr = boardVar.value<BoardWeakPtr>();
        boardData = weakPtr.lock();
    }

    // get a pixmap of the board's stored icon
    const QIcon boardIcon { decrole.value<QIcon>() };
    QPixmap pixmap { boardIcon.pixmap(ICONDISPLAYWIDTH, ICONDISPLAYHEIGHT) };

    // this is our entire drawing area to work with
    QRect iconCellRect { option.rect };
    iconCellRect.setWidth(LISTICONWIDTH);
    iconCellRect.setHeight(LISTICONHEIGHT);

    // this it the `QRect` in which we will draw the board's icon
    QRect iconRect = pixmap.rect();
    const std::int32_t hCenterAdjust = (option.rect.width() / 2) - (ICONDISPLAYWIDTH / 2);
    const std::int32_t vCenterAdjust = (option.rect.height() / 2) - (ICONDISPLAYHEIGHT / 2);
    iconRect.moveLeft(iconCellRect.left() + hCenterAdjust);
    iconRect.moveTop(iconCellRect.top() + vCenterAdjust);

    if (boardData && boardData->getStatus() == owl::Board::OFFLINE)
    {
        QImage boardImg = pixmap.toImage().convertToFormat(QImage::Format_Grayscale8);
        painter->drawImage(iconRect, boardImg);
    }
    else
    {
        painter->drawPixmap(iconRect, pixmap);
    }

    if (option.state & QStyle::State_Selected)
    {
        auto padding = (iconCellRect.height() - iconRect.height()) / 2;
        padding += 4;

        QRect selectRect { iconCellRect };
        selectRect.moveRight(selectRect.left() + 1);
        selectRect.adjust(0, padding, 0, -padding);

        QPen pen(QBrush(QColor("#A0A0A0")), 6);
        painter->setPen(pen);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawRoundRect(selectRect);
    }

    if (option.state & QStyle::State_MouseOver)
    {
        QPen pen(QBrush(QColor("lightgrey")), 3);
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
//* BoardIconView
//********************************

BoardIconView::BoardIconView(QWidget* parent /* = 0*/)
    : QWidget(parent),
      _logger { owl::initializeLogger("BoardIconView") }
{
    _iconModel = new QStandardItemModel;
    _iconModel->setColumnCount(1);

    std::int32_t idx = 0;
    const auto boardlist = owl::BoardManager::instance()->getBoardList();
    for (const auto& board : boardlist)
    {
        _logger->trace("Adding {} ({}) at index {}",
            board->getName().toStdString(), board->getUsername().toStdString(), idx);

        QByteArray buffer(board->getFavIcon().toLatin1());
        const QImage originalImage = QImage::fromData(QByteArray::fromBase64(buffer));
        const QImage overlayImage { ":/icons/error_32.png" };

        QImage resultImg = overlayImages(originalImage, QImage{});
        QIcon finalIcon { QPixmap::fromImage(resultImg) };

        QStandardItem* item = new QStandardItem(finalIcon, QString{});
        item->setToolTip(board->getName());
        item->setTextAlignment(Qt::AlignCenter);

        QVariant dataVar = QVariant::fromValue(std::weak_ptr<owl::Board>(board));
        item->setData(dataVar, BOARDPTR_ROLE);

        _iconModel->insertRow(idx++, item);
    }

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

    const std::int32_t width = LISTICONWIDTH;
    const std::int32_t height = LISTICONHEIGHT;
    _listView->setIconSize(QSize(width, height));

    _listView->setItemDelegate(new BoardIconViewDelegate);
    _listView->setModel(_iconModel);

    QObject::connect(_listView, &QWidget::customContextMenuRequested,
        [this](const QPoint &pos) { this->doContextMenu(pos); });

    QObject::connect(_listView, &BoardIconListView::onIndexChanged,
        [this](const QModelIndex& index)
        {
            QVariant boardVar = index.data(BOARDPTR_ROLE);
            if (!boardVar.isNull() && boardVar.isValid())
            {
                Q_EMIT onCurrentBoardChanged(boardVar.value<BoardWeakPtr>());
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

        if (boardPtr->getStatus() == Board::OFFLINE)
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

        if (boardPtr->getStatus() == Board::ONLINE)
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
                    onDeleteBoard(boardVar.value<BoardWeakPtr>());
                });
        }

//        menu->popup(this->mapToGlobal(pos));
        menu->exec(this->mapToGlobal(pos));
    }
//    this->ind

}

} // namespace
