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
    const auto mousePt = event->position().toPoint();

    if (rect().contains(mousePt))
    {
        Q_EMIT clicked();
    }
}

} // namespace
