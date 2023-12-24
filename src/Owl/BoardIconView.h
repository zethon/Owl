#pragma once

#include <memory>

#include <QStyledItemDelegate>
#include <QWidget>
#include <QListView>

#include "Data/Board.h"
#include "Data/BoardManager.h"

class QStandardItemModel;
class QVBoxLayout;

namespace spdlog
{
    class logger;
}

namespace owl
{

class ConnectionListModel;
using SpdLogPtr = std::shared_ptr<spdlog::logger>;

}

namespace owl
{

class BoardIconViewDelegate : public QStyledItemDelegate
{
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class BoardIconView : public QWidget
{

Q_OBJECT

public:
    BoardIconView(QWidget* parent = nullptr);
    virtual ~BoardIconView() = default;

    void setConnectionFile(const QString& cf) { _connectionFile = cf; }
    void initListView();

Q_SIGNALS:
    void onAddNewBoard();
    void onAddNewWebBrowser();
    void onBoardClicked(owl::BoardWeakPtr);
    void onBoardDoubleClicked(owl::BoardWeakPtr);
    void onConnectBoard(owl::BoardWeakPtr);
    void onMarkBoardRead(owl::BoardWeakPtr);
    void onEditBoard(owl::BoardWeakPtr);
    void onCopyBoardAddress(owl::BoardWeakPtr);
    void onOpenBoardInBrowser(owl::BoardWeakPtr);
    void onDeleteBoard(owl::BoardWeakPtr);

private:
    void doContextMenu(const QPoint &pos);
    void requestBoardDelete(BoardWeakPtr board);
    void createListView();

    QString                 _connectionFile;
    ConnectionListModel*    _connectionModel = nullptr;
    owl::Board*             _rawBoardPtr = nullptr;
    QListView*              _listView = nullptr;
    owl::SpdLogPtr          _logger;
};

}
