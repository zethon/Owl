#pragma once
#include <QFrame>

namespace owl
{

class ConnectionFrame : public QFrame
{

public:

    ConnectionFrame(const std::string& uuid, QWidget *parent = nullptr)
        : QFrame(parent)
        , _uuid(uuid)
    {
        this->setObjectName(QString::fromStdString(uuid));
    }

    virtual ~ConnectionFrame() = default;

    std::string uuid() const { return _uuid; }


private:
    std::string _uuid;
};

} // namespace owl
