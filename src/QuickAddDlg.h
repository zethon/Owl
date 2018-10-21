// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <memory>
#include <log4qt/logger.h>
#include "ui_QuickAddDlg.h"

namespace Ui
{
	class QuickAddDlg;
}

namespace owl
{

class Board;
using BoardPtr = std::shared_ptr<Board>;

class ConfiguringBoardDlg;

class QuickAddDlg : public QDialog, public Ui::QuickAddDlg
{
	Q_OBJECT
	LOG4QT_DECLARE_QCLASS_LOGGER
	
public:
	~QuickAddDlg() = default;

	QuickAddDlg(QWidget *parent = 0);
	
    BoardPtr getNewBoard() const;
	void setParserByName(const QString& name);

protected Q_SLOTS:
	void onConfigureFinished(int result);

Q_SIGNALS:
	void newBoardAddedEvent(BoardPtr);

protected:
	virtual void accept();
    
private:
	ConfiguringBoardDlg* _configureDlg;
};

} //namespace owl
