#pragma once
#include <QtGui>
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
	
public:
	AboutDlg(QWidget *parent = nullptr);
	virtual ~AboutDlg();

protected Q_SLOTS:
	virtual void linkClicked(const QUrl& url);
};

} //namespace owl
