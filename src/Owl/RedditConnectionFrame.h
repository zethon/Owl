#pragma once

#include <QLabel>

#include "ConnectionFrame.h"

namespace owl
{

class RedditConnectionFrame : public owl::ConnectionFrame
{
    Q_OBJECT

public:
    RedditConnectionFrame(const std::string& uuid, QWidget *parent = nullptr);

Q_SIGNALS:

private:
    QLabel* _label;
};

} // namespace owl
