#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include "../Parsers/Forum.h"

namespace owl
{

using ForumPtr = std::shared_ptr<owl::Forum>;
using ForumWeakPtr = std::weak_ptr<owl::Forum>;

class ForumTreeItem
{
public:
    using ItemPtr = std::shared_ptr<ForumTreeItem>;
    using ItemUniquePtr = std::unique_ptr<ForumTreeItem>;
    using ItemList = std::vector<ItemUniquePtr>;

    explicit ForumTreeItem(ForumPtr data, ForumPtr parent = nullptr);
    ~ForumTreeItem() = default;

    ItemList& getChildren() { return _children; }

private:
    ItemList                _children;
    ForumTreeItem*          _parent;

//public:
//    explicit TreeItem(const QList<QVariant> &data, TreeItem *parentItem = 0);
//    ~TreeItem();

//    void appendChild(TreeItem *child);

//    TreeItem *child(int row);
//    int childCount() const;
//    int columnCount() const;
//    QVariant data(int column) const;
//    int row() const;
//    TreeItem *parentItem();

//private:
//    QList<TreeItem*> m_childItems;
//    QList<QVariant> m_itemData;
//    TreeItem *m_parentItem;
};

class ForumTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ForumTreeModel(const QString &data, QObject *parent = 0);
    ~ForumTreeModel();


private:
    std::shared_ptr<owl::Forum>     _rootForum;
//    QVariant data(const QModelIndex &index, int role) const override;
//    Qt::ItemFlags flags(const QModelIndex &index) const override;
//    QVariant headerData(int section, Qt::Orientation orientation,
//                        int role = Qt::DisplayRole) const override;
//    QModelIndex index(int row, int column,
//                      const QModelIndex &parent = QModelIndex()) const override;
//    QModelIndex parent(const QModelIndex &index) const override;
//    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
//    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

//private:
//    void setupModelData(const QStringList &lines, TreeItem *parent);

//    TreeItem *rootItem;
};

} // namespace
