#pragma once

#include <QLabel>

#include "ConnectionFrame.h"

namespace owl
{

class ChatConnectionFrame : public owl::ConnectionFrame
{
    Q_OBJECT

public:
    ChatConnectionFrame(QWidget *parent = nullptr);

Q_SIGNALS:

private:
    QLabel* _label;
};

} // namespace owl
