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

class LogoView : public QWidget
{
public:
    ~LogoView() = default;
    explicit LogoView(QWidget* parent = nullptr);

private:
    QLabel*  _bgImg;

};

class ContentView : public QWidget
{

Q_OBJECT

public:
    virtual ~ContentView() = default;
    ContentView(QWidget* parent = nullptr);

private:
    LogoView*               _logoView;
    owl::SpdLogPtr          _logger;
};

}
