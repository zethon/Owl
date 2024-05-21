#pragma once
#include <QQuickWidget>

namespace owl
{

class ConnectionFrame : public QQuickWidget
{

public:

    ConnectionFrame(const std::string& uuid, QWidget *parent = nullptr)
        : QQuickWidget(parent)
        , _uuid(uuid)
    {
        this->setObjectName(QString::fromStdString(uuid));
    }

    virtual ~ConnectionFrame() = default;

    std::string uuid() const { return _uuid; }

    virtual void initFocus(Qt::FocusReason reason)
    {
        this->setFocus(reason);
    }

private:
    std::string _uuid;
};

} // namespace owl
