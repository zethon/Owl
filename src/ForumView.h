#pragma once

#include <memory>
#include <QWidget>

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

    owl::SpdLogPtr          _logger;
};

}
