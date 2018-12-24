#include <queue>

#include <QAbstractItemModel>

#include "ForumTreeModel.h"

namespace owl
{

void crawlForums(ForumPtr forum, std::vector<ForumPtr>& nodes)
{
    auto& children = forum->getChildren();
    for (auto& child : children)
    {
        auto childForum = std::dynamic_pointer_cast<owl::Forum>(child);
        if (childForum)
        {
            nodes.push_back(childForum);
            crawlForums(childForum, nodes);
        }
    }
}

ForumTreeModel::ForumTreeModel(const ForumPtr root, QObject *parent)
    : QAbstractItemModel(parent)
{
    crawlForums(root, _nodes);
}

QModelIndex ForumTreeModel::index(int row, int column, const QModelIndex & parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    Q_ASSERT(!parent.isValid());
    Q_ASSERT(column == 0);

    std::size_t trow = static_cast<std::size_t>(row);
    return createIndex(row, column, _nodes.at(trow).get());
}

QModelIndex ForumTreeModel::parent(const QModelIndex & index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int ForumTreeModel::rowCount(const QModelIndex & parent) const
{
    if (parent.column() > 0) return 0;
    return static_cast<int>(_nodes.size());
}

int ForumTreeModel::columnCount(const QModelIndex&  parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant ForumTreeModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid()) return QVariant{};

    if (role == Qt::DisplayRole)
    {
        owl::Forum* item = static_cast<owl::Forum*>(index.internalPointer());
        QString pre; 
        for (auto x = 0; x < item->getLevel(); ++x)
        {
            pre += ".";
        }
        return pre + item->getName();
    }
    else if (role == Qt::ForegroundRole)
    {
       return QColor(Qt::lightGray);
    }
    else if (role == Qt::ToolTipRole)
    {
        owl::Forum* item = static_cast<owl::Forum*>(index.internalPointer());
        return item->getName();
    }

    return QVariant{};
}

} // namespace
