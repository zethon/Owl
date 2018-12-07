#include "Core.h"
#include "ErrorReportDlg.h"

namespace owl
{

ErrorReportDlg::ErrorReportDlg(const OwlException& ex,
        QString errorTitle, 
        QString errorMessage,
        ErrorActionType actionType,
        QWidget* parent)
	: QDialog(parent),
	  _errorTitle(errorTitle),
	  _errorDetailsHtml(errorMessage),
	  _actionType(actionType)
{
	init();
}

ErrorReportDlg::ErrorReportDlg(const OwlException& ex, QWidget* parent)
	: ErrorReportDlg(ex, QString(), QString(), ErrorActionType::OK, parent)
{
	// do nothing
}

ErrorReportDlg::ErrorReportDlg(QString strTitle, const OwlException& ex)
	: ErrorReportDlg(ex, strTitle, QString(), ErrorActionType::OK, nullptr)
{
    // do nothing
}

ErrorReportDlg::ErrorReportDlg()
    : ErrorReportDlg(OwlException{}, QString(), QString(), ErrorActionType::OK, nullptr)
{
	// do nothing
}

ErrorReportDlg::ErrorReportDlg(QWidget *parent)
    : ErrorReportDlg(OwlException{}, QString(), QString(), ErrorActionType::OK, parent)
{
	// do nothing
}

ErrorReportDlg::ErrorReportDlg(QString strTitle, QString strError)
    : ErrorReportDlg(OwlException{}, strTitle, strError, ErrorActionType::OK, nullptr)
{
	// do nothing
}

void ErrorReportDlg::displayException(LuaParserException* lex)
{
	_errorTitle = "Lua Parser Error";

	_errorDetailsHtml  = QString(
		"<b>Location</b>: %1(%2)<br/>"
		"<b>Error</b>:<blockquote>%3</blockquote><br/>")
		.arg(lex->filename())
		.arg(lex->line())
		.arg(lex->message());

	//QString keyDump;
	//owl::StringMap* params = const_cast<StringMap*>(lex->getParams());
	//
	//if (params != NULL && params->getPairs() != NULL && params->getPairs()->size() > 0)
	//{
	//	QHash<QString, QString>::const_iterator i;
	//	for (i = params->getPairs()->begin(); i != params->getPairs()->end(); ++i)
	//	{
	//		QString newText = QString("<b>%1</b>: <pre>%2</pre><br/>")
	//							.arg(i.key())
	//							.arg(i.value());

	//		keyDump.append(newText);
	//	}
	//}

	//errorText.append("<hr/>");
	//errorText.append(keyDump);

	//this->errorDetails->setHtml(errorText);
}

void ErrorReportDlg::displayException(WebException* ex)
{
	_errorTitle = "Web Exception";

	_errorDetailsHtml = QString(
		"Url:&nbsp;<b>%1</b><br/>"
		"HTTP Response:&nbsp;<b>%2</b><br/>"
		"Error:<br/><b><i>%3</i></b><br/>")
		.arg(ex->url())
		.arg(ex->statuscode())
		.arg(ex->message());
}

void ErrorReportDlg::onOKClicked()
{
    this->close();
}

void ErrorReportDlg::init()
{
	// set up the ui objects in the dialog
	setupUi(this);

	// set the window title
	QString windowTitle = QString("%1 Error").arg(APP_TITLE);
	this->setWindowTitle(windowTitle);

	// adding the sad face icon
	this->picLabel->setPixmap(QPixmap(":/images/sad.png"));

	// set up signals
	QObject::connect(OKBtn,SIGNAL(clicked()), this, SLOT(onOKClicked()));
	QObject::connect(cancelBtn,SIGNAL(clicked()), this, SLOT(onCancelClicked()));

	//// if we have an error object, get the info we want from that
	//if (_error != nullptr)
	//{
	//	if (dynamic_cast<owl::LuaParserException*>(_error.get()) != nullptr)
	//	{
	//		displayException(dynamic_cast<owl::LuaParserException*>(_error.get()));
	//	}
	//	else if (dynamic_cast<owl::WebException*>(_error.get()) != nullptr)
	//	{
	//		displayException(dynamic_cast<owl::WebException*>(_error.get()));
	//	}
	//	else if (!_error->message().isEmpty())
	//	{
	//		_errorDetailsHtml = _error->message();
	//		_errorTitle = "An unexpected error has occurred";
	//	}
	//	else
	//	{
	//		_errorDetailsHtml = "There was an unknown error.";
	//	}
	//}	

	QString actionLabel;
	switch (_actionType)
	{
		case ErrorActionType::SUBMITCANEL:
			actionLabel = "Owl encountered an error. The details below can be sent to Owl to improve product performance by clicking SUBMIT.";
			this->cancelBtn->setVisible(true);
		break;

		case ErrorActionType::OK:
		default:
			actionLabel = "Owl encountered an error. The details of the error are below.";
			this->cancelBtn->setVisible(false);
		break;
	}

	// set the text items
	actionLbl->setText(actionLabel);
	
    appendStackTrace();
    errorDetails->setHtml(_errorDetailsHtml);
	errorTitle->setText(_errorTitle);
}

void ErrorReportDlg::appendStackTrace()
{
    //if (!_error) return;

    //if (auto st = boost::get_error_info<owl::traced>(*_error); st)
    //{
    //    std::stringstream ss;
    //    ss << *st;

    //    const QString trace =
    //        QString("<pre>%1</pre>").arg(QString::fromStdString(ss.str()));

    //    _errorDetailsHtml.append(trace);
    //}
}

void ErrorReportDlg::onCancelClicked()
{
    OWL_THROW_EXCEPTION(NotImplementedException("ErrorReportDlg::onCancelClicked() is not implemented"));
}

} // namespace
