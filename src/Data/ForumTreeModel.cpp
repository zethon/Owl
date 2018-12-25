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

QImage resizeImage1(const QImage& original, const QSize& size)
{
    QImage finalImage { original };
    qreal iXScale = static_cast<qreal>(size.width()) / static_cast<qreal>(finalImage.width());
    qreal iYScale = static_cast<qreal>(size.height()) / static_cast<qreal>(finalImage.height());
    if (iXScale > 1 || iXScale < 1 || iYScale > 1 || iYScale < 1)
    {
        QTransform transform;
        transform.scale(iXScale, iYScale);
        finalImage = finalImage.transformed(transform, Qt::SmoothTransformation);
    }

    return finalImage;
}

QVariant ForumTreeModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid()) return QVariant{};

    if (role == Qt::DecorationRole)
    {
        const QSize iconSize(12,12);
        owl::Forum* item = static_cast<owl::Forum*>(index.internalPointer());

        if (item->getForumType() == owl::Forum::ForumType::FORUM)
        {
            QImage image(":/icons/forum.png");
            return resizeImage1(image, iconSize);
        }
        else if (item->getForumType() == owl::Forum::ForumType::LINK)
        {
            QImage image(":/icons/link.png");
            return resizeImage1(image, iconSize);
        }
    }
    if (role == Qt::DisplayRole)
    {
        owl::Forum* item = static_cast<owl::Forum*>(index.internalPointer());
        return item->getName();
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
    else if (role == Qt::TextAlignmentRole)
    {
        owl::Forum* item = static_cast<owl::Forum*>(index.internalPointer());
        if (item->getForumType() == owl::Forum::ForumType::CATEGORY
                && index.model()->hasIndex(index.row() - 1, index.column()))
        {
            auto prevIdx = index.model()->index(index.row() - 1, index.column());
            owl::Forum* previtem = static_cast<owl::Forum*>(prevIdx.internalPointer());
            if (previtem->getForumType() == owl::Forum::ForumType::FORUM)
            {
                return static_cast<int>(Qt::AlignLeft | Qt::AlignBottom);
            }
        }
    }


    return QVariant{};
}

} // namespace
