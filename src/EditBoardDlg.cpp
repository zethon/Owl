#include <QMessageBox>
#include "Data/BoardManager.h"
#include "EditBoardDlg.h"

using namespace Log4Qt;

namespace owl
{

EditBoardDlg::EditBoardDlg(BoardPtr board, QWidget* parent)
	: QDialog(parent),
    _board(board)
{
	setupUi(this);
	this->tabWidget->setCurrentIndex(0);

	setFixedSize(this->width(),this->height());

	boardName->setText(_board->getName());
	boardUrl->setText(_board->getUrl());

	boardUsername->setText(_board->getUsername());
	boardPassword->setText(_board->getPassword());

	parserLbl->setText(_board->getParser()->getPrettyName());

	autoLoginCB->setChecked(_board->isAutoLogin());
    
	showImgsCB->setChecked(_board->getOptions()->getBool("showImages", false));
    
    enableAutoRefresh->setChecked(_board->getOptions()->getBool("enableAutoRefresh", true));

    refreshRateTB->setText(_board->getOptions()->getText("refreshRate"));
	refreshRateTB->setValidator(new QIntValidator(this));

    threadsPPTB->setText(_board->getOptions()->getText("threadsPerPage"));
    threadsPPTB->setValidator(new QIntValidator(this));
    threadsPPTB->setDisabled(board->getParser()->defaultThreadsPerPage().second);

    postsPPTB->setText(_board->getOptions()->getText("postsPerPage"));
	postsPPTB->setValidator(new QIntValidator(this));
    postsPPTB->setDisabled(board->getParser()->defaultPostsPerPage().second);

	QByteArray buffer(_board->getFavIcon().toLatin1());
	QImage image = QImage::fromData(QByteArray::fromBase64(buffer));

	if (image.width() != 32 || image.height() != 32)
	{
		// calculate the scaling factor based on wanting a 32x32 image
		qreal iXScale = (qreal)32 / (qreal)image.width();
		qreal iYScale = (qreal)32 / (qreal)image.height();

		QTransform transform;
		transform.scale(iXScale, iYScale);
		image = image.transformed(transform, Qt::SmoothTransformation);
	}

	QIcon icon(QPixmap::fromImage(image));
	this->iconLbl->setPixmap(QPixmap::fromImage(image));
    
    refreshUserAgentField();

	QObject::connect(showPasswordCB,SIGNAL(clicked()), this, SLOT(onShowPasswordClicked()));
    QObject::connect(useCustomUserAgent, SIGNAL(clicked()), this, SLOT(onToggleCustomUserAgent()));
    QObject::connect(useDefaultUserAgent, SIGNAL(clicked()), this, SLOT(onToggleCustomUserAgent()));
	QObject::connect(autoConfigBtn, SIGNAL(clicked()), this, SLOT(onAutoConfigureClicked()));

    renderPluginSettings();
    
    _oldValues = _board->getBoardData();
}

EditBoardDlg::~EditBoardDlg()
{
	// do nothing
}
    
void EditBoardDlg::onToggleCustomUserAgent()
{
    // set the board object's settings
    _board->setCustomUserAgent(useCustomUserAgent->isChecked());
    _board->setUserAgent(userAgentTB->text());
    
    // update the UI
    refreshUserAgentField();
}

void EditBoardDlg::refreshUserAgentField()
{
    bool bUseCustomUserAgent = _board->getCustomUserAgent();
    useCustomUserAgent->setChecked(bUseCustomUserAgent);
    useDefaultUserAgent->setChecked(!bUseCustomUserAgent);
    userAgentTB->setEnabled(bUseCustomUserAgent);
    userAgentTB->setText(_board->getUserAgent());
    
    if (bUseCustomUserAgent)
    {
            userAgentTB->setStyleSheet("QLineEdit { font: normal }");
    }
    else
    {
        userAgentTB->setStyleSheet("QLineEdit { font: italic }");
    }
}
    
void EditBoardDlg::accept()
{
	if (boardName->text().size() == 0)
	{
		QMessageBox* warning = new QMessageBox(
			QMessageBox::Warning,
			tr("Invalid Message Board Name"),
            "The message board name cannot be blank",
			QMessageBox::Ok,
            this, 
			Qt::Sheet);

		warning->open();

		return;
	}

	QUrl qUrl(boardUrl->text());
	if (!qUrl.isValid())
	{
		QMessageBox* warning = new QMessageBox(
			QMessageBox::Warning,
			tr("Invalid Message Board Url"),
            "The message board Url you entered is invalid",
			QMessageBox::Ok,
            this, 
			Qt::Sheet);

		warning->open();

		return;
	}

	_board->setName(boardName->text());
	_board->setUrl(boardUrl->text());


	_board->setUsername(boardUsername->text());
	_board->setPassword(boardPassword->text());
    
	_board->setAutoLogin(autoLoginCB->isChecked());
	_board->getOptions()->setOrAdd("showImages", showImgsCB->isChecked());
	_board->getOptions()->setOrAdd("enableAutoRefresh", enableAutoRefresh->isChecked());
     
    _board->getOptions()->setOrAdd("refreshRate", refreshRateTB->text());
    _board->getOptions()->setOrAdd("threadsPerPage", threadsPPTB->text());
    _board->getOptions()->setOrAdd("postsPerPage", postsPPTB->text());
    
    _board->setCustomUserAgent(useCustomUserAgent->isChecked());
    _board->setUserAgent(userAgentTB->text());
    
    writePluginSettings();
    
	if (BoardManager::instance()->updateBoard(_board))
	{
		_board->refreshOptions();
		Q_EMIT boardSavedEvent(_board, _oldValues);
	}
    else
    {
        QMessageBox::warning(this, "Database Error", "Could not save changes.");
    }

	QDialog::accept();
}

void EditBoardDlg::onShowPasswordClicked()
{
	if (showPasswordCB->checkState() == Qt::Checked)
	{
		boardPassword->setEchoMode(QLineEdit::Normal);
	}
	else
	{
		boardPassword->setEchoMode(QLineEdit::Password);		
	}
}

void EditBoardDlg::onBoardNameChanged(QString text)
{
	if (text.size() > 0)
	{
		this->setWindowTitle(QString("Board: %1").arg(text));
	}
	else
	{
		this->setWindowTitle(QString("[No Name]"));
	}
}
    
void EditBoardDlg::renderPluginSettings()
{
	StringMapPtr bo = _board->getOptions();

    try
    {
        useEncryptionCB->setChecked(bo->getBool(Board::Options::USE_ENCRYPTION));
        encryptionKey->setText(bo->getText(Board::Options::ENCKEY));
        encryptionSeed->setText(bo->getText(Board::Options::ENCSEED));
    }
    catch (const owl::OwlException&)
    {
        bo->add(Board::Options::USE_ENCRYPTION, (bool)false);
        bo->add(Board::Options::ENCSEED, (QString)"");
        bo->add(Board::Options::ENCKEY, (QString)"");

        useEncryptionCB->setChecked(false);
        encryptionKey->setText("");
        encryptionSeed->setText("");
    }
}

void EditBoardDlg::writePluginSettings()
{
	StringMapPtr bo = _board->getOptions();

	bo->setOrAdd(Board::Options::USE_ENCRYPTION, (bool)(useEncryptionCB->checkState() == Qt::Checked));
	bo->setOrAdd(Board::Options::ENCKEY, (QString)encryptionKey->text());
	bo->setOrAdd(Board::Options::ENCSEED, (QString)encryptionSeed->text());
}

void EditBoardDlg::onAutoConfigureClicked()
{
	ParserBasePtr parser = _board->getParser();
	
	configStatusLbl->setText(tr("Requesting encryption settings..."));
	StringMap s = parser->getEncryptionSettings();

	if (s.has("success") && s.getBool("success"))
	{
		configStatusLbl->setText(tr("Encryption detected! Settings set successfully!"));
		useEncryptionCB->setChecked(true);
		encryptionKey->setText(s.getText("key"));
		encryptionSeed->setText(s.getText("seed"));
	}
	else
	{
		configStatusLbl->setText(tr("Either there are no encryption settings configured for this board or the board administrator has disabled auto-configuration for encyrption.\r\n\r\nContact the board administrator for more information."));
	}
}

} // end namespace
