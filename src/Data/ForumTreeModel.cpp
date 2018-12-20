#include "ForumTreeModel.h"

namespace owl
{

Q_INVOKABLE QModelIndex ForumTreeModel::index(int row, int column, const QModelIndex & parent) const
{
    return Q_INVOKABLE QModelIndex();
}

Q_INVOKABLE QModelIndex ForumTreeModel::parent(const QModelIndex & child) const
{
    return Q_INVOKABLE QModelIndex();
}

Q_INVOKABLE int ForumTreeModel::rowCount(const QModelIndex & parent) const
{
    return Q_INVOKABLE int();
}

Q_INVOKABLE int ForumTreeModel::columnCount(const QModelIndex & parent) const
{
    return Q_INVOKABLE int();
}

Q_INVOKABLE QVariant ForumTreeModel::data(const QModelIndex & index, int role) const
{
    return Q_INVOKABLE QVariant();
}

} // namespace