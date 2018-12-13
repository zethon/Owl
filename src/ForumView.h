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

using SpdLogPtr = std::shared_ptr<spdlog::logger>;

class ForumView : public QWidget
{

Q_OBJECT

public:
    virtual ~ForumView() = default;
    ForumView(QWidget* parent = nullptr);

    void loadBoard(const owl::BoardPtr);

private:
    QLabel*                 _tempLabel;
    QTreeView*              _treeView;

    owl::SpdLogPtr          _logger;
};

}
