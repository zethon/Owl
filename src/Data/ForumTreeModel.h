#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include "../Parsers/Forum.h"

#define SHAREDPTR_ROLE       Qt::UserRole+1

namespace owl
{

using ForumPtr = std::shared_ptr<owl::Forum>;
using ForumWeakPtr = std::weak_ptr<owl::Forum>;

class ForumTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ForumTreeModel(const ForumPtr root, QObject *parent = nullptr);
    ~ForumTreeModel() = default;

private:

    // Inherited via `QAbstractItemModel`
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex & index) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

    std::vector<ForumPtr>   _nodes;
};

} // namespace
