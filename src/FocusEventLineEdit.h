#pragma once
#include <QLineEdit>
#include <log4qt/logger.h>

namespace owl
{

class FocusEventLineEdit : public QLineEdit
{
	Q_OBJECT
	LOG4QT_DECLARE_QCLASS_LOGGER

public:
	FocusEventLineEdit(QWidget* parent = 0);
	virtual ~FocusEventLineEdit();

Q_SIGNALS:
	void focusChangedEvent(bool bHasFocus);

protected:
    virtual void focusInEvent(QFocusEvent *e) override;
    virtual void focusOutEvent(QFocusEvent *e) override;
};

} // namespace
