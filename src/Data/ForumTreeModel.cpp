#include <queue>

#include <QAbstractItemModel>

#include "ForumTreeModel.h"

namespace owl
{

ForumTreeModel::ForumTreeModel(const ForumPtr root, QObject *parent)
    : QAbstractItemModel(parent)
{
    std::queue<owl::ForumPtr> nodes;

    nodes.push(root);

    while(not nodes.empty())
    {
        auto nodePtr = nodes.front();
        nodes.pop();

        if (!nodePtr->IsRoot()) _nodes.push_back(nodePtr);

        if (auto& children = nodePtr->getChildren(); children.size() > 0)
        {
            for (auto& child : children)
            {
                auto childForum = std::dynamic_pointer_cast<owl::Forum>(child);
                if (childForum) nodes.push(childForum);
            }
        }
    }
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

    QVariant data;

    if (role == Qt::DisplayRole)
    {
        owl::Forum* item = static_cast<owl::Forum*>(index.internalPointer());


//        switch (index.column())
//        {
////            // icone
////            case 0:
////            {
////                QIcon icon { ":/icons/owl_128.png" };
////                data = QVariant{icon.pixmap(QSize(16,16))};
////            }
////            break;

//            case 0:
                data = QVariant::fromValue(item->getName());
//            break;

////            case 1:
//                data = QVariant{"Other"};
////            break;
//        }
    }
    else if (role == Qt::ForegroundRole)
    {
        data = QColor(Qt::lightGray);
    }
    else if (role == Qt::DecorationRole)
    {
//        qDebug() << "DECOR ROLE";
    }

    return data;
}

} // namespace
