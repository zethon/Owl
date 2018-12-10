#pragma once

#include <memory>
#include <QQuickWidget>

namespace spdlog
{
    class logger;
}

namespace owl
{

using SpdLogPtr = std::shared_ptr<spdlog::logger>;

class BoardIconTree : public QQuickWidget
{

Q_OBJECT

public:
	BoardIconTree(QWidget* parent = 0);
	virtual ~BoardIconTree() = default;

private:
    owl::SpdLogPtr _logger;

};

}
