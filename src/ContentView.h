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

using SpdLogPtr = std::shared_ptr<spdlog::logger>;

class ContentView : public QWidget
{

Q_OBJECT

public:
    virtual ~ContentView() = default;
    ContentView(QWidget* parent = nullptr);

private:
    owl::SpdLogPtr          _logger;
};

}
