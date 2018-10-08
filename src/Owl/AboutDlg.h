// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <QtGui>
#include <logger.h>
#include "ui_AboutDlg.h"

namespace Ui
{  
	class AboutDlg;
}

namespace owl
{
	
class AboutDlg : public QDialog, public Ui::AboutDlg
{
	Q_OBJECT
	LOG4QT_DECLARE_QCLASS_LOGGER
	
public:
	AboutDlg(QWidget *parent = nullptr);
	virtual ~AboutDlg();

protected Q_SLOTS:
	virtual void linkClicked(const QUrl& url);
};

} //namespace owl
