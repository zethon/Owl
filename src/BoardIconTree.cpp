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

#define ICONWIDTH       64
#define ICONHEIGHT      64

namespace owl
{

QIcon bufferToIcon(const char* buf)
{
    QByteArray buffer(buf);
    QImage image = QImage::fromData(QByteArray::fromBase64(buffer));

    // calculate the scaling factor based on wanting a 32x32 image
    qreal iXScale = (qreal)ICONWIDTH / (qreal)image.width();
    qreal iYScale = (qreal)ICONHEIGHT / (qreal)image.height();

    // only scale the image if it's not the right size
    if (iXScale != 1 || iYScale != 1)
    {
        QTransform transform;
        transform.scale(iXScale, iYScale);
        image = image.transformed(transform, Qt::SmoothTransformation);
    }

    return QIcon { QPixmap::fromImage(image) };
}

BoardIconTree::BoardIconTree(QWidget* parent /* = 0*/)
    : _logger { owl::initializeLogger("BoardIconTree") }
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

    _listView->setModel(_iconModel);

    QVBoxLayout* layout = new QVBoxLayout;

    layout->addWidget(_listView);

    setLayout(layout);
}


} // namespace
