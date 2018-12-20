#include "ForumTreeModel.h"

namespace owl
{

//ForumTreeItem::ItemPtr ForumTreeItem::child(std::size_t idx)
//{
//    if (idx >= _children.size())
//    {
//        return ForumTreeItem::ItemPtr{};
//    }

//    return _children.at(idx);
//}


ForumTreeModel::ForumTreeModel(const ForumPtr root, QObject *parent)
    : QAbstractItemModel(parent),
      _root(root)
{}

QModelIndex ForumTreeModel::index(int row, int column, const QModelIndex & parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    owl::Forum* parentItem;
    if (parent.isValid())
    {
        parentItem = static_cast<owl::Forum*>(parent.internalPointer());
    }
    else
    {
        parentItem = _root.get();
    }

    QModelIndex retval;

    auto& children = parentItem->getForums();
    if (row < children.size())
    {
        retval = createIndex(row, column, children.at(row).get());
    }

    return retval;
}

QModelIndex ForumTreeModel::parent(const QModelIndex & child) const
{
    return Q_INVOKABLE QModelIndex();
}

int ForumTreeModel::rowCount(const QModelIndex & parent) const
{
    if (parent.column() > 0) return 0;

    int count = 0;

    owl::ForumPtr forum;

    if (parent.isValid())
    {
        count = static_cast<owl::Forum*>(parent.internalPointer())->getChildren().size();
    }
    else
    {
        count = _root->getChildren().size();
    }

    return count;
}

int ForumTreeModel::columnCount(const QModelIndex&  parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant ForumTreeModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid()) return QVariant{};

    if (role != Qt::DisplayRole) return QVariant{};

    owl::Forum* item = static_cast<owl::Forum*>(index.internalPointer());

    QVariant data;

    switch (index.column())
    {
        // icone
        case 0:
            data = QVariant{"ICON"};
        break;

        case 1:
            data = QVariant::fromValue(item->getName());
        break;

        case 2:
            data = QVariant{"Other"};
        break;
    }

    return data;
}

} // namespace
