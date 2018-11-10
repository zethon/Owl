#pragma once
#include <QLineEdit>

namespace owl
{

class FocusEventLineEdit : public QLineEdit
{
	Q_OBJECT

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
