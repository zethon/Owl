#include <QWidget>
#include <QListView>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>
#include <QPainter>
#include <QPixmap>

#include  <Utils/OwlLogger.h>

#include "Data/BoardManager.h"
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

    const QIcon boardIcon { decrole.value<QIcon>() };

    painter->save();

    // this is our entire area to work with
    QRect iconCellRect { option.rect };
    iconCellRect.setWidth(LISTICONWIDTH);
    iconCellRect.setHeight(LISTICONHEIGHT);

    // draw the board's icon
    QPixmap pixmap { boardIcon.pixmap(ICONDISPLAYWIDTH, ICONDISPLAYHEIGHT) };
    QRect iconRect = pixmap.rect();
    const std::int32_t hCenterAdjust = (option.rect.width() / 2) - (ICONDISPLAYWIDTH / 2);
    const std::int32_t vCenterAdjust = (option.rect.height() / 2) - (ICONDISPLAYHEIGHT / 2);

    iconRect.moveLeft(iconCellRect.left() + hCenterAdjust);
    iconRect.moveTop(iconCellRect.top() + vCenterAdjust);
    painter->drawPixmap(iconRect, pixmap);

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
        if (idx % 2) resultImg = resultImg.convertToFormat(QImage::Format_Grayscale8);
        QIcon finalIcon { QPixmap::fromImage(resultImg) };

        QStandardItem* item = new QStandardItem(finalIcon, QString{});
        item->setToolTip(board->getName());
        item->setTextAlignment(Qt::AlignCenter);

        _iconModel->insertRow(idx++, item);
    }

    _listView = new QListView(this);
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

    const std::int32_t width = LISTICONWIDTH;
    const std::int32_t height = LISTICONHEIGHT;
    _listView->setIconSize(QSize(width, height));

//    _listView->setIconSize(QSize(LISTICONWIDTH,LISTICONHEIGHT));


    _listView->setItemDelegate(new BoardIconViewDelegate);
    _listView->setModel(_iconModel);


    QVBoxLayout* layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget(_listView);

//    QPushButton* pb = new QPushButton(this);
//    pb->setText("CLICK");
//    QObject::connect(pb, &QPushButton::clicked,
//        [this]()
//        {

//        });

    setLayout(layout);

    qDebug() << "SIZE: " << _listView->iconSize();
}

} // namespace
