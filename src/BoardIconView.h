#pragma once

#include <memory>

#include <QStyledItemDelegate>
#include <QWidget>
#include <QListView>

#include "Data/Board.h"
#include "Data/BoardManager.h"

#define ICONTYPE_ROLE       Qt::UserRole+1
#define BOARDPTR_ROLE       Qt::UserRole+2

class QStandardItemModel;
class QVBoxLayout;

namespace spdlog
{
    class logger;
}

namespace owl
{

using SpdLogPtr = std::shared_ptr<spdlog::logger>;

enum class IconType
{
    ADDICON, BOARDICON
};

}

Q_DECLARE_METATYPE(owl::IconType);

namespace owl
{

class BoardIconViewDelegate : public QStyledItemDelegate
{
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class BoardIconListView : public QListView
{

Q_OBJECT

public:
    using QListView::QListView;

Q_SIGNALS:
    void onIndexChanged(const QModelIndex&);

protected:
    void currentChanged(const QModelIndex&, const QModelIndex&) override;
};

class BoardIconModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit BoardIconModel(QObject *parent = nullptr);
    ~BoardIconModel() = default;

private:
    // Inherited via `QAbstractItemModel`
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

    BoardManagerPtr     _boardManager;
};

class BoardIconView : public QWidget
{

Q_OBJECT

public:
    BoardIconView(QWidget* parent = nullptr);
    virtual ~BoardIconView() = default;

    void loadBoards();

Q_SIGNALS:
    void onAddNewBoard();
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
    void initListView();

    void requestBoardDelete(BoardWeakPtr board);

    BoardIconListView*      _listView = nullptr;
    owl::SpdLogPtr          _logger;
};

}
