#pragma once

#include <memory>

#include <QStyledItemDelegate>
#include <QWidget>

class QListView;
class QStandardItemModel;
class QVBoxLayout;

namespace spdlog
{
    class logger;
}

namespace owl
{

using SpdLogPtr = std::shared_ptr<spdlog::logger>;

class BoardIconViewDelegate : public QStyledItemDelegate
{
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class BoardIconTree : public QWidget
{

Q_OBJECT

public:
    BoardIconTree(QWidget* parent = nullptr);
    virtual ~BoardIconTree() = default;

private:
    QListView*              _listView;
    QStandardItemModel*     _iconModel;

    owl::SpdLogPtr          _logger;
};

}
