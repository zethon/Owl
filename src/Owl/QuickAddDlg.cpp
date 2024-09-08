// Owl - www.owlclient.com
// Copyright (c) 2012-2023, Adalid Claure <aclaure@gmail.com>

#include <Parsers/ParserManager.h>
#include "ConfiguringBoardDlg.h"
#include "QuickAddDlg.h"

namespace owl
{

// *****************************************************************************************
// THIS IS OLD CODE AND MAY NEED TO BE DELETED
// *****************************************************************************************

QuickAddDlg::QuickAddDlg(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
    setStyleSheet("background-color:white;");
    setFixedSize(this->width(),this->height());

#ifdef Q_WS_WIN
    this->setWindowIcon(QIcon(":/icons/owl_256.png"));
#endif

	QPixmap banner;
    banner.load(":/icons/owl_128.png");
	logoLbl->setPixmap(banner);

	QPixmap error;
	error.load(":/icons/error_arrow.png");
	urlError->setPixmap(error);
	urlError->setVisible(false);
	usernameError->setPixmap(error);
	usernameError->setVisible(false);

	passwordError->setPixmap(error);
	passwordError->setVisible(false);

	parserError->setPixmap(error);
	parserError->setVisible(false);

    auto parserList = PARSERMGR->getParsers();
    auto keyList = parserList.keys();
    std::sort(keyList.begin(), keyList.end());

    for (const auto& k : keyList)
    {
        this->parserCombo->addItem(parserList[k].prettyName, parserList[k].name);
    }

    this->parserCombo->setInsertPolicy(QComboBox::InsertAtTop);
    this->parserCombo->insertSeparator(0);
    this->parserCombo->insertItem(0, tr("Auto-detect"), QString("#AUTODETECT"));
    this->parserCombo->setCurrentIndex(0);

	guestCB->setVisible(false);
}

void QuickAddDlg::accept()
{
    QString strError;
    QUrl	url = QUrl::fromUserInput(urlTB->text());

	if (!url.isValid())
	{
		strError.append("Please enter a valid URL. ");
		urlError->setVisible(true);
	}
    else if (url.scheme() != "http" && url.scheme() != "https")
	{
		strError.append("Please enter a valid web address. ");
		urlError->setVisible(true);
	}
	else
	{
		urlError->setVisible(false);
	}

	if (usernameTB->text().isEmpty())
	{
		strError.append("Please enter a valid username. ");
		usernameError->setVisible(true);
	}
	else
	{
		usernameError->setVisible(false);
	}

	if (passwordTB->text().isEmpty())
	{
		strError.append("Please enter a valid password.");
		passwordError->setVisible(true);
	}
	else
	{
		passwordError->setVisible(false);
	}

    const QVariant vdata = parserCombo->itemData(parserCombo->currentIndex());
    const QString parser = vdata.value<QString>();

	if (!strError.isEmpty())
	{
		messageLbl->setStyleSheet("QLabel { color: red; }");
		messageLbl->setText(strError);
		return;
	}

	_configureDlg = new ConfiguringBoardDlg(this);
	_configureDlg->setInfo(urlTB->text(), usernameTB->text(),
		passwordTB->text(), parser,
		guestCB->checkState() == Qt::Checked);

	connect(_configureDlg, SIGNAL(finished(int)), this, SLOT(onConfigureFinished(int)));
	connect(_configureDlg, SIGNAL(newBoardAddedEvent(BoardPtr)), this, SIGNAL(newBoardAddedEvent(BoardPtr)));

	_configureDlg->open();
	_configureDlg->start();
}

void QuickAddDlg::onConfigureFinished(int)
{
	if (_configureDlg->getSuccess())
	{
		// since ConfiguringBoardDlg passes the newBoardEvent through this dialog,
		// this should never happen
		QDialog::accept();
	}
	else
	{
		messageLbl->setStyleSheet("QLabel { color: red; }");

		QString errorText(_configureDlg->getErrorString());

		if (errorText.isEmpty())
		{
			errorText = tr("There was an unknown error adding the new message board.");
		}

		messageLbl->setText(errorText);
	}

    _configureDlg->deleteLater();
    _configureDlg = nullptr;
}

void QuickAddDlg::setParserByName(const QString& name)
{
	int iParser = parserCombo->findData(name);

	if (iParser >= 0)
	{
		parserCombo->setCurrentIndex(iParser);
	}
}

// *****************************************************************************************
// END OF THE OLD CODE THAT MAY NEED TO BE DELETED
// *****************************************************************************************

} // end namespace
