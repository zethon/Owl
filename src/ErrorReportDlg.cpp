#include "Core.h"
#include "ErrorReportDlg.h"

namespace owl
{

ErrorReportDlg::ErrorReportDlg(const OwlException& ex, QWidget* parent)
    : ErrorReportDlg(ex, QString(), QString(), ErrorActionType::OK, parent)
{}

ErrorReportDlg::ErrorReportDlg(const OwlException& ex, QString strTitle)
    : ErrorReportDlg(ex, strTitle, QString(), ErrorActionType::OK, nullptr)
{}

ErrorReportDlg::ErrorReportDlg(const OwlException& ex,
        QString errorTitle, 
        QString errorMessage,
        ErrorActionType actionType,
        QWidget* parent)
	: QDialog(parent),
	  _errorTitle(errorTitle),
	  _errorDetailsHtml(errorMessage),
	  _actionType(actionType),
      _logger(owl::initializeLogger("ErrorReportDlg"))
{
    init(ex);
}


void ErrorReportDlg::onOKClicked()
{
    this->close();
}

void ErrorReportDlg::init(const OwlException& ex)
{
	// set up the ui objects in the dialog
	setupUi(this);

	// set the window title
    setWindowTitle(QString{APP_TITLE});

	// adding the sad face icon
    picLabel->setPixmap(QPixmap(":/images/sad.png"));

	// set up signals
	QObject::connect(OKBtn,SIGNAL(clicked()), this, SLOT(onOKClicked()));
	QObject::connect(cancelBtn,SIGNAL(clicked()), this, SLOT(onCancelClicked()));

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

    appendDetails(ex);
    errorDetails->setHtml(_errorDetailsHtml);
	errorTitle->setText(_errorTitle);
}

void ErrorReportDlg::appendDetails(const OwlException& ex)
{
    const QString header = QString(R"(<big><b>%1</b></big>)").arg(ex.message());
    _errorDetailsHtml.append(header);

    const QString details = QString(R"(
<hr/>
<pre>
%1
</pre>
)").arg(ex.details());

    _errorDetailsHtml.append(details);
}

void ErrorReportDlg::onCancelClicked()
{
    OWL_THROW_EXCEPTION(NotImplementedException("ErrorReportDlg::onCancelClicked() is not implemented"));
}

} // namespace
