// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <QLabel>

namespace owl
{

class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    ClickableLabel(QWidget* parent = 0);
    virtual ~ClickableLabel() = default;

Q_SIGNALS:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent * event);
};

} // namespace
