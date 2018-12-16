#pragma once

#include <memory>
#include <QWidget>

class QLabel;
class QTreeView;

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


class ForumView : public QWidget
{

Q_OBJECT

public:
    virtual ~ForumView() = default;
    ForumView(QWidget* parent = nullptr);

     void doBoardClicked(const owl::BoardWeakPtr);

private:
    QLabel*                 _tempLabel;
    QTreeView*              _treeView;

    owl::BoardWeakPtr       _currentBoard;
    owl::SpdLogPtr          _logger;
};

}
