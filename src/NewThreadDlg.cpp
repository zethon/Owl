#include <QtCore>
#include <QtGui>
#include <QMessageBox>
#include <Parsers/ParserBase.h>
#include <Utils/Settings.h>
#include "Data/Board.h"
#include "NewThreadDlg.h"

namespace owl
{

/////////////////////////////////////////////////////////////////////////
// NewThreadDlg
/////////////////////////////////////////////////////////////////////////

NewThreadDlg::NewThreadDlg(BoardItemPtr itemParent, NewItemType type, QWidget* parent /*= nullptr*/)
	: QDialog(parent),
	_type(type),
    _itemParent(itemParent),
    _board(itemParent->getBoard())
{
	setupUi(this);

    auto board = _board.lock();
    if (!board)
    {
        OWL_THROW_EXCEPTION(OwlException("Board object is null"));
    }

     _parser = board->cloneParser();

    leftBtn->setIcon(QIcon(":/icons/justifyleft.gif"));
    centerBtn->setIcon(QIcon(":/icons/justifycenter.gif"));
    rightBtn->setIcon(QIcon(":/icons/justifyright.gif"));
    
    // schedule this dialog to be deleted when it's closed
    QObject::connect(this, SIGNAL(finished(int)), this, SLOT(deleteLater()));

    // "accepted" is called from a successful submission of a thread/post and when that
    // happens we want to autmatically close the dialog
    QObject::connect(this, SIGNAL(accepted()), this, SLOT(deleteLater()));

    // aka the "Submit" button
    QObject::connect(saveButton, SIGNAL(clicked()), this, SLOT(onSubmitClicked()));

    // clicking the "Cancel" button will invoke QDialog::closeEvent() which will then
    // close the dialog or ask the user if they're sure that they want to cllose it
    QObject::connect(cancelButton, &QPushButton::clicked, [this]() { this->close(); });

	// set the actions for the dialog's editing tools
    QObject::connect(boldBtn, &QPushButton::clicked, [this]() { encloseText("B"); });
    QObject::connect(italicBtn, &QPushButton::clicked, [this]() { encloseText("I"); });
    QObject::connect(underlineBtn, &QPushButton::clicked, [this]() { encloseText("U"); });
    QObject::connect(leftBtn, &QPushButton::clicked, [this]() { encloseText("LEFT"); });
    QObject::connect(centerBtn, &QPushButton::clicked, [this]() { encloseText("CENTER"); });
    QObject::connect(rightBtn, &QPushButton::clicked, [this]() { encloseText("RIGHT"); });
    QObject::connect(imgBtn, &QPushButton::clicked, [this]() { encloseText("IMG"); });
    QObject::connect(urlBtn, &QPushButton::clicked, [this]() { encloseText("URL"); });

    QObject::connect(titleText, &SpellCheckEdit::focusChangedEvent,
        [this](bool bHasFocus)
        {
            if (bHasFocus)
            {
                titleLbl->setStyleSheet("QLabel { color: rgb(56, 146, 254); }");
                titleText->setFrameShape(QFrame::StyledPanel);
                titleText->setFrameShadow(QFrame::Plain);
            }
            else
            {
                titleLbl->setStyleSheet("QLabel { color: rgb(199, 192, 192); }");
                titleText->setFrameShape(QFrame::NoFrame);
            }
        });

    QObject::connect(tagsTxt, &FocusEventLineEdit::focusChangedEvent,
        [this](bool bHasFocus)
        {
            if (bHasFocus)
            {
                tagsLbl->setStyleSheet("QLabel { color: rgb(56, 146, 254); }");
            }
            else
            {
                tagsLbl->setStyleSheet("QLabel { color: rgb(199, 192, 192); }");
            }
        });

    // the parser will throw errors upon submitting the thread or post, so connect to that handler
    // so we can display those errors here in this dialog
    QObject::connect(_parser.get(), SIGNAL(errorNotification(OwlExceptionPtr)), this, SLOT(showError(OwlExceptionPtr)));

    // set the font for the editor
    QFont editorFont(SettingsObject().read("editor.font.family").toString());
    editorFont.setPointSize(SettingsObject().read("editor.font.size").toInt());
    postTxt->setFont(editorFont);

    // TODO: should probably a be per parser limit
    titleText->setMaxLength(256);
}

NewThreadDlg::NewThreadDlg(ForumPtr itemParent, QWidget *parent)
    : NewThreadDlg(itemParent, NEWTHREAD, parent)
{
    // If the parent is a forum then we are creating a new thread
    ForumPtr f = _itemParent->upCast<ForumPtr>();

    const auto board = _board.lock();
    if (!board)
    {
        OWL_THROW_EXCEPTION(OwlException("Board object is null"));
    }

    const QString strTitle = QString(tr("%1 | New thread in forum '%2'"))
        .arg(board->getName())
        .arg(f->getName());

    setWindowTitle(strTitle);

    titleText->setPlaceholderText(tr("New thread title"));
    titleText->setFocus();

    _type = NEWTHREAD;

    QObject::connect(_parser.get(), &owl::ParserBase::submitNewThreadCompleted,
        [this](ThreadPtr t)
        {
            auto board = _board.lock();

            if (board)
            {
                _bSubmitted = true;
                statusImgLbl->setPixmap(QPixmap(":/icons/success_32.png"));
                statusLbl->setText(tr("Success!"));
                accepted();
                board->newThreadEvent(t);
            }
        });
}

NewThreadDlg::NewThreadDlg(ThreadPtr itemParent, QWidget *parent)
    : NewThreadDlg(itemParent, NEWPOST, parent)
{
    // If the parent is a forum then we are creating a new post

    const auto board = _board.lock();
    if (!board)
    {
        OWL_THROW_EXCEPTION(OwlException("Board object is null"));
    }

    const QString strTitle
        {
            QString(tr("%1 | New post in thread '%2'"))
                .arg(board->getName())
                .arg(_itemParent->getTitle())
        };

    setWindowTitle(strTitle);

    titleText->setPlaceholderText(tr("New post title"));

    tagsLbl->setVisible(false);
    tagsTxt->setVisible(false);
    postTxt->setFocus();

    _type = NEWPOST;

    QObject::connect(_parser.get(), &owl::ParserBase::submitNewPostCompleted,
        [this](PostPtr p)
        {
            auto board = _board.lock();

            if (board)
            {
                _bSubmitted = true;
                statusImgLbl->setPixmap(QPixmap(":/icons/success_32.png"));
                statusLbl->setText(tr("Success!"));
                accepted();

                board->newPostEvent(p);
            }
        });
}

NewThreadDlg::~NewThreadDlg()
{
    const auto board = _board.lock();
    if (board)
    {
        board->disconnect(this);
    }
}

void NewThreadDlg::lockForm()
{
    titleText->setReadOnly(true);
    tagsTxt->setReadOnly(true);
    postTxt->setReadOnly(true);

    saveButton->setEnabled(false);
    cancelButton->setEnabled(false);
}

void NewThreadDlg::unlockForm()
{
    titleText->setReadOnly(false);
    tagsTxt->setReadOnly(false);
    postTxt->setReadOnly(false);

    saveButton->setEnabled(true);
    cancelButton->setEnabled(true);
}

void NewThreadDlg::setQuoteText(const QString& text)
{
    postTxt->setPlainText(text);
    postTxt->moveCursor(QTextCursor::End);
}

void NewThreadDlg::encloseText(QString tag)
{
    QTextCursor txtCursor = postTxt->textCursor();
    
    if (txtCursor.hasSelection())
    {
        QString selectTxt = QString("[%1]%2[/%1]")
            .arg(tag)
            .arg(txtCursor.selectedText());
        
        txtCursor.insertText(selectTxt);
    }
    else
    {
        txtCursor.insertText(QString("[%1][/%1]").arg(tag));
        txtCursor.setPosition(txtCursor.position() - (tag.length() + 3));
        postTxt->setTextCursor(txtCursor);
    }
}

void NewThreadDlg::closeEvent(QCloseEvent* e)
{
    bool bDoAccept = true;

    // _bSutmitted will be true if the user has submitted the thread/post and it has
    // returned back successfully and now we're trying to automatically close the
    // dialog. If _bSubmitted is false, then see if the title or text documents have
    // been modified, and if so, ask the user if they're sure they want to close the
    // dialog
    if (!_bSubmitted &&
            (postTxt->document()->isModified() || titleText->document()->isModified()))
	{
        const QString strMsg = QString(tr("You haven't finished your post yet. Do you want to close "
			"without submitting?\n\nAre you sure you want to close this post?"));

		QMessageBox* messageBox = new QMessageBox(
			QMessageBox::Question,
			tr("Confirm closing"),
			strMsg,
			QMessageBox::Yes | QMessageBox::No,
			this,
			Qt::Sheet);

		messageBox->setWindowModality(Qt::WindowModal);

        bDoAccept = messageBox->exec() == QMessageBox::Yes;
	}

    if (bDoAccept)
    {
        deleteLater();
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

ThreadPtr NewThreadDlg::getNewThread()
{
	ThreadPtr ret(new Thread(""));
    ret->setTitle(titleText->document()->toPlainText());

	PostPtr fp(new Post(""));
	fp->setText(postTxt->toPlainText());

    // TODO: this is setting the parent to _forumParent when it feels
    // like this should be set to the thread object just created (ret)
    // *Maybe* ret should be set to _forumParent in the event of
    // a NEWPOST and the new post gets appended to the list of posts
	//
	// NOTE: This was changed from _forumParent to _itemParent
	_itemParent->addChild(fp);

	ret->getPosts().push_back(fp);

	QStringList taglist = tagsTxt->text().split(",");
	for (QString tag : taglist)
	{
		tag = tag.simplified();

		// hardcoded support for a 25 char limit
		if (!tag.isEmpty() && tag.size() < 25)
		{
            ret->pushTag(tag);
		}
	}

	_itemParent->addChild(ret);
	return ret;
}

void NewThreadDlg::keyPressEvent(QKeyEvent *e)
{
    if(e->key() != Qt::Key_Escape)
    {
        QDialog::keyPressEvent(e);
    }
}

void NewThreadDlg::onSubmitClicked()
{
    Q_ASSERT(_type == NewThreadDlg::NEWPOST || _type == NewThreadDlg::NEWTHREAD);

    lockForm();

    statusLbl->setText(tr("Sending..."));

    // start the loading movie
    auto workingMovie = new QMovie(":/icons/loading.gif", QByteArray(), this);
    statusImgLbl->setMovie(workingMovie);
    workingMovie->start();

    auto board = _itemParent->getBoard();
    if (_type == NewThreadDlg::NEWPOST)
    {
        const PostPtr post = getNewThread()->getPosts().at(0);
        _parser->submitNewPostAsync(post);
    }
    else
    {
        _parser->submitNewThreadAsync(getNewThread());
    }
}

void NewThreadDlg::showError(OwlExceptionPtr ex)
{
    _bSubmitted = false;
    unlockForm();

    // clear the status window
    statusImgLbl->setPixmap(QPixmap());
    statusLbl->setText("");

    const QString strTitle { tr("Error submitting post") };

    QMessageBox msgBox(
        QMessageBox::Warning,
        strTitle,
        ex->message(),
        QMessageBox::Ok,
        this,
        Qt::Sheet);

    msgBox.exec();
}

} // namespace
