// Owl - www.owlclient.com
// Copyright Adalid Claure <aclaure@gmail.com>

#pragma once
#include <Parsers/LuaParserBase.h>
#include "ui_ErrorReportDlg.h"

namespace Ui
{
	class ErrorReportDlg;
}

namespace owl
{

class ErrorReportDlg : public QDialog, public Ui::ErrorReportDlg
{
	Q_OBJECT
	
public:
	typedef enum { NEWPOST, NEWTHREAD } NewItemType;
	typedef enum { SUBMITCANEL, OK } ErrorActionType;

    virtual ~ErrorReportDlg() = default;

    ErrorReportDlg();
    ErrorReportDlg(QWidget *parent);
    ErrorReportDlg(QString strTitle, QString strError);
    ErrorReportDlg(QString strTitle, const OwlException& ex);
    ErrorReportDlg(const OwlException& ex, QWidget* parent);

	ErrorReportDlg(
		const OwlException& ex,
		QString errorTitle, 
		QString errorMessage,
		ErrorActionType actionType,
		QWidget* parent);
    
protected Q_SLOTS:
	void onOKClicked();
	void onCancelClicked();

private:
	void displayException(LuaParserException* lex);
	void displayException(WebException* ex);
    void appendStackTrace();

	void init();

	QString				_errorTitle;
	QString				_errorDetailsHtml;

	ErrorActionType		_actionType;
};

} // namespace
