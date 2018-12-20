#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include "../Parsers/Forum.h"

namespace owl
{

using ForumPtr = std::shared_ptr<owl::Forum>;
using ForumWeakPtr = std::weak_ptr<owl::Forum>;

//class ForumTreeItem
//{
//public:
//    using ItemPtr = std::shared_ptr<ForumTreeItem>;
//    using ItemList = std::vector<ItemPtr>;

//    explicit ForumTreeItem(ForumPtr data, ForumPtr parent = nullptr);
//    ~ForumTreeItem() = default;

//    std::size_t childCount() const { return _children.size(); }
//    ItemPtr child(std::size_t idx);

//    ForumPtr forum() { return _forumPtr; }


//private:
//    ForumPtr                _forumPtr;

//    ItemList                _children;
//    ForumTreeItem*          _parent;
//};

class ForumTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ForumTreeModel(const ForumPtr root, QObject *parent = nullptr);
    ~ForumTreeModel() = default;

private:

    // Inherited via `QAbstractItemModel`
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex & child) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

//    ForumTreeItem::ItemPtr root;
    owl::ForumPtr   _root;
};

} // namespace
