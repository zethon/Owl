// Owl - www.owlclient.com
// Copyright Adalid Claure <aclaure@gmail.com>

#pragma once
#include <logger.h>
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
	LOG4QT_DECLARE_QCLASS_LOGGER
	
public:
	typedef enum { NEWPOST, NEWTHREAD } NewItemType;
	typedef enum { SUBMITCANEL, OK } ErrorActionType;

	ErrorReportDlg(
		OwlExceptionPtr ex,
		QString errorTitle, 
		QString errorMessage,
		ErrorActionType actionType,
		QWidget* parent);

	ErrorReportDlg(
		OwlExceptionPtr ex, 
		QWidget* parent);

	ErrorReportDlg();
	ErrorReportDlg(QWidget *parent);
	ErrorReportDlg(QString strTitle, QString strError);
	ErrorReportDlg(QString strTitle, OwlExceptionPtr ex);

	virtual ~ErrorReportDlg();
    
protected Q_SLOTS:
	void onOKClicked();
	void onCancelClicked();

private:
	void displayException(LuaParserException* lex);
	void displayException(WebException* ex);

	void init();

	QString				_errorTitle;
	QString				_errorDetailsHtml;
	OwlExceptionPtr		_error;

	ErrorActionType		_actionType;
};

} // namespace
