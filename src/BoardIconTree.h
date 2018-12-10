#pragma once

#include <memory>

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

class BoardIconTree : public QWidget
{

Q_OBJECT

public:
    BoardIconTree(QWidget* parent = 0);
    virtual ~BoardIconTree() = default;

private:
    QListView*              _listView;
    QStandardItemModel*     _iconModel;

    owl::SpdLogPtr          _logger;
};

}
