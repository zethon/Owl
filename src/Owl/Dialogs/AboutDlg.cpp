// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#include "../Core.h"
#include "AboutDlg.h"

using namespace Log4Qt;

namespace owl
{

////////////////////////////////////////////////////////
// class methods
		
AboutDlg::AboutDlg(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);

    this->setWindowTitle(tr("About ") + QStringLiteral(APP_NAME));
    imageLabel->setPixmap(QPixmap(":/images/owl_64.png"));
	titleLbl->setText(APP_NAME);
	versionLbl->setText("Version " OWL_VERSION);
    copyrightLbl->setText(COPYRIGHT);
    detailsLbl->setText(QString("Built %1").arg(OWL_VERSION_DATE_TIME));
    qtInfoLbl->setText(QString("Qt %1 based forum reader.").arg(QT_VERSION_STR));

    tabWidget->setCurrentIndex(0);
}

AboutDlg::~AboutDlg()
{
	// do nothing
}

void AboutDlg::linkClicked( const QUrl& url )
{
	QDesktopServices::openUrl(url);
}
    
} // end namespace
