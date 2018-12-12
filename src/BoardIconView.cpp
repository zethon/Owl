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
#define LISTICONHEIGHT        70

#define ICONDISPLAYWIDTH      38
#define ICONDISPLAYHEIGHT     38

namespace owl
{

static const char* itemStyleSheet = R"(
QListView
{
    background: #181F26;
    border-style: none;
        padding-left: 0px;
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
\
    const QIcon boardIcon { decrole.value<QIcon>() };
    const QString boardText { index.data(Qt::DisplayRole).toString() };


    qDebug() << "boardIcon       : " << boardIcon;
    qDebug() << "boardText       : " << boardText;
    qDebug() << "option.rect     : " << option.rect;
    qDebug() << "option.x        : " << option.rect.x();
    qDebug() << "option.y        : " << option.rect.y();
    qDebug() << "option.height   : " << option.rect.height();
    qDebug() << "option.width    : " << option.rect.width();

    if (option.state & QStyle::State_MouseOver) qDebug() << "MOUSE OVER";
    if (option.state & QStyle::State_Selected) qDebug() << "SELECTED";

    painter->save();

    QRect iconCellRect { option.rect };

    QPixmap pixmap { boardIcon.pixmap(ICONDISPLAYWIDTH, ICONDISPLAYHEIGHT) };
    QRect iconRect = pixmap.rect();
    const std::int32_t hCenterAdjust = (option.rect.width() / 2) - (ICONDISPLAYWIDTH / 2);
    const std::int32_t vCenterAdjust = (option.rect.height() / 2) - (ICONDISPLAYHEIGHT / 2);

    iconRect.moveLeft(iconCellRect.left() + hCenterAdjust);
    iconRect.moveTop(iconCellRect.top() + vCenterAdjust);
    painter->drawPixmap(iconRect, pixmap);

    if (option.state & QStyle::State_Selected)
    {
        // draw icon container border
        painter->setPen(QColor("red"));

        iconCellRect.adjust(0,1,-1,-10);
        painter->drawRect(iconCellRect);

//        painter->draw
    }
    else if (option.state & QStyle::State_MouseOver)
    {
        // draw icon container border
        QPen pen(QBrush(QColor("lightgrey")), 3);
        painter->setPen(pen);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawRoundRect(iconRect);
    }

//    iconRect.adjust(17, 17, 0, 0);

//    iconRect.setTopLeft(QPoint(option.rect.x(), option.rect.y()));
//    auto rightSide = option.rect.x() + pixmap.rect().width();
//    auto bottomSide = option.rect.y() + pixmap.rect().height();
//    iconRect.setBottomRight(QPoint(rightSide, bottomSide));

//    QRect iconRect = pixmap.rect();
////    iconRect.adjust(15,15,-15,-15);
//    iconRect.moveTop(option.rect.top());
//    iconRect.adjust(0,0,0,-10);

//    qDebug() << "iconRect: " << iconRect;

//    painter->setPen(QColor("red"));
//    painter->fillRect(iconRect, QColor("red"));

//    QRect drawRect = pixmap.rect();

//    QRect drawRect { option.rect };

//    auto width = (drawRect.width() / 2) - ()

//    drawRect.adjust(10,10,-10,-10);

//    painter->fillRect(drawRect, QColor(255,0,0));
//    painter->setPen(QColor("yellow"));
//    painter->drawRect(drawRect);


//    QVariant var = index.data(Qt::DecorationRole);
//    QIcon icon = var.value<QIcon>();
//    qDebug() << var;

//    painter->save();

//    QRect rect { option.rect };
//    rect.moveBottom(rect.bottom() - 10);
////    rect.moveRight(-10);
//    painter->drawPixmap(rect,icon.pixmap(ICONSCALEWIDTH,ICONSCALEHEIGHT));


//    auto border = option.rect;
//    border.moveBottom(-10);

//////    painter->save();
//    painter->drawRect(border);

//////    QImage image = index.data().value<QImage>();
////    QPixmap pm = option.icon.pixmap(40,40);
////    QImage image = pm.toImage();
////    painter->drawImage(0,0,image);
////    painter->restore();
//////

////    option.Middle

    painter->restore();
//    QStyledItemDelegate::paint(painter, option, index);
}

QSize BoardIconViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // padding between the icons
    return QStyledItemDelegate::sizeHint(option, index);
//        + QSize{0, 10};
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

        QImage resultImg = overlayImages(originalImage, overlayImage);
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
    layout->setSpacing(100);
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
