#include <QWidget>
#include <QListView>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

#include  <Utils/OwlLogger.h>

#include "Data/BoardManager.h"
#include "BoardIconTree.h"

#define ICONWIDTH       256
#define ICONHEIGHT      256

namespace owl
{

static const char* itemStyleSheet = R"(
QListView
{
    background: #444444;
}

QListView::item::selected
{
    background: #00FF00;
}

QListView::item::hover
{
    background: #FF00FF;
}
)";

QIcon bufferToIcon(const char* buf)
{
    QByteArray buffer(buf);
    QImage image = QImage::fromData(QByteArray::fromBase64(buffer));

    // calculate the scaling factor based on wanting a 32x32 image
    qreal iXScale = static_cast<qreal>(ICONWIDTH) / static_cast<qreal>(image.width());
    qreal iYScale = static_cast<qreal>(ICONHEIGHT) / static_cast<qreal>(image.height());

    // only scale the image if it's not the right size
    if (iXScale > 1 || iXScale < 1 || iYScale > 1 || iYScale < 1)
    {
        QTransform transform;
        transform.scale(iXScale, iYScale);
        image = image.transformed(transform, Qt::SmoothTransformation);
    }

    return QIcon { QPixmap::fromImage(image) };
}

BoardIconTree::BoardIconTree(QWidget* parent /* = 0*/)
    : QWidget(parent),
      _logger { owl::initializeLogger("BoardIconTree") }
{
    _iconModel = new QStandardItemModel;
    _iconModel->setColumnCount(1);

    std::size_t idx = 0;
    const auto boardlist = owl::BoardManager::instance()->getBoardList();
    for (const auto& board : boardlist)
    {
        _logger->trace("Adding {} ({}) at index {}",
            board->getName().toStdString(), board->getUsername().toStdString(), idx);

        const QIcon icon = bufferToIcon(board->getFavIcon().toLatin1());
        QStandardItem* item = new QStandardItem(icon, QString{});
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

    _listView->setModel(_iconModel);

    QVBoxLayout* layout = new QVBoxLayout;

    layout->addWidget(_listView);

    setLayout(layout);
}


} // namespace
