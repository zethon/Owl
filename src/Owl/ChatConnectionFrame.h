#pragma once

#include <QLabel>
#include <QQmlEngine>
#include <QQuickItem>

#include "ConnectionFrame.h"

namespace owl
{

class ChatConnectionFrame : public owl::ConnectionFrame
{
    Q_OBJECT

public:
    ChatConnectionFrame(QWidget *parent = nullptr);

    void initFocus(Qt::FocusReason reason) override
    {
        this->owl::ConnectionFrame::setFocus(reason);
        QMetaObject::invokeMethod(this->rootObject(), "setFocus");
    }

Q_SIGNALS:
    void doSetFocus();

private:
    QLabel* _label;
};

} // namespace owl
