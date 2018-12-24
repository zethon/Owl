#pragma once

#include <memory>

#include <QWidget>
#include <QStyledItemDelegate>
#include <QListView>

class QLabel;

namespace spdlog
{
    class logger;
}

namespace owl
{

class Board;
using BoardPtr = std::shared_ptr<Board>;
using BoardWeakPtr = std::weak_ptr<Board>;

using SpdLogPtr = std::shared_ptr<spdlog::logger>;

class ForumViewDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    ~ForumViewDelegate() = default;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class ForumListControl : public QListView
{
public:
    using QListView::QListView;

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
};

class ForumView : public QWidget
{

Q_OBJECT

public:
    virtual ~ForumView() = default;
    ForumView(QWidget* parent = nullptr);

     void doBoardClicked(const owl::BoardWeakPtr);

private:
    QLabel*                 _boardLabel;
    QLabel*                 _userLabel;
    QLabel*                 _userImgLabel;

    ForumListControl*       _listView;

    owl::BoardWeakPtr       _currentBoard;
    owl::SpdLogPtr          _logger;
};

}
