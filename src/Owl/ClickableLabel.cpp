// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#include <QMouseEvent>
#include "ClickableLabel.h"

namespace owl
{

ClickableLabel::ClickableLabel(QWidget* parent)
    : QLabel(parent)
{
    // do nothing
}

void ClickableLabel::mouseReleaseEvent(QMouseEvent* event)
{
    const auto mousePt = event->localPos().toPoint();

    if (rect().contains(mousePt))
    {
        Q_EMIT clicked();
    }
}

} // namespace
