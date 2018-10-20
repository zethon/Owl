// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#include "FocusEventLineEdit.h"

namespace owl
{

/////////////////////////////////////////////////////////////////////////
// FocusEventLineEdit
/////////////////////////////////////////////////////////////////////////

FocusEventLineEdit::FocusEventLineEdit(QWidget* parent)
	: QLineEdit(parent)
{
}

FocusEventLineEdit::~FocusEventLineEdit()
{
}

void FocusEventLineEdit::focusInEvent(QFocusEvent *e)
{
	QLineEdit::focusInEvent(e);
	Q_EMIT focusChangedEvent(true);
}

void FocusEventLineEdit::focusOutEvent(QFocusEvent *e)
{
	QLineEdit::focusOutEvent(e);
	Q_EMIT focusChangedEvent(false);
}

} // namespace
