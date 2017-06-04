// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <log4qt/logger.h>
#include "Data/Board.h"
#include "ui_EditBoardDlg.h"

namespace Ui
{
	class EditBoardDlg;
}

namespace owl
{

class EditBoardDlg : public QDialog, public Ui::EditBoardDlg
{
	Q_OBJECT
	LOG4QT_DECLARE_QCLASS_LOGGER
	
public:
	EditBoardDlg(BoardPtr board, QWidget *parent = 0);
	~EditBoardDlg();

Q_SIGNALS:
    void boardSavedEvent(BoardPtr board, StringMap oldValues);

protected Q_SLOTS:
	void onShowPasswordClicked();
	void onAutoConfigureClicked();
	void onBoardNameChanged(QString text);
    void onToggleCustomUserAgent();

protected:
	virtual void accept();

private:
    void refreshUserAgentField();
    
    void renderPluginSettings();
    void writePluginSettings();
    
    BoardPtr            _board;
    StringMap           _oldValues;
};

} //namespace owl
