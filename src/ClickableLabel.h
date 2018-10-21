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
