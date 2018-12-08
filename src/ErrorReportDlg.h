// Owl - www.owlclient.com
// Copyright Adalid Claure <aclaure@gmail.com>

#pragma once
#include <Parsers/LuaParserBase.h>
#include <Utils/OwlLogger.h>

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

    ErrorReportDlg(const OwlException& ex, QString strTitle);
    ErrorReportDlg(const OwlException& ex, QWidget* parent);

protected Q_SLOTS:
	void onOKClicked();
	void onCancelClicked();

private:
    ErrorReportDlg(
        const OwlException& ex,
        QString errorTitle,
        QString errorMessage,
        ErrorActionType actionType,
        QWidget* parent);

    void displayException(LuaParserException* lex);
	void displayException(WebException* ex);

    void init(const OwlException& ex);
    void appendStackTrace(const OwlException& ex);

	QString				_errorTitle;
	QString				_errorDetailsHtml;

	ErrorActionType		_actionType;

    SpdLogPtr           _logger;
};

} // namespace
