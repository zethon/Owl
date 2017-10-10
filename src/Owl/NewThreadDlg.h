// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <QtCore>
#include <QtGui>
#include <QPlainTextEdit>
#include <log4qt/logger.h>
#include <Parsers/Forum.h>
#include "Controls/PostTextEditor.h"
#include "ui_NewThreadDlg.h"

namespace Ui
{
	class NewThreadDlg;
}

namespace owl
{

class ParserBase;
using ParserBasePtr = std::shared_ptr<ParserBase>;

/////////////////////////////////////////////////////////////////////////
// NewThreadDlg
/////////////////////////////////////////////////////////////////////////
//
// Creates the dialog for composing a new post. This class can be used
// to create new threads or new posts in a thread. This class will use
// QObject::deleteLater() to schedule itself for deletion when it's
// closed.
//
/////////////////////////////////////////////////////////////////////////
	
class NewThreadDlg : public QDialog, public Ui::NewThreadDlg
{
	Q_OBJECT
	LOG4QT_DECLARE_QCLASS_LOGGER
	
	typedef enum { NEWPOST, NEWTHREAD } NewItemType;

public:
	
    NewThreadDlg(ForumPtr itemParent, QWidget* parent = nullptr);
    NewThreadDlg(ThreadPtr itemParent, QWidget* parent = nullptr);
	
    ~NewThreadDlg();

	ThreadPtr getNewThread();
	//BoardItemPtr getForumParent() const { return _forumParent; }
	BoardItemPtr getItemParent() const { return _itemParent; }

	void setQuoteText(const QString& text);

Q_SIGNALS:
    void onItemSubmitted(BoardPtr, BoardItemPtr);

protected:
    virtual void keyPressEvent(QKeyEvent *e) override;
	virtual void closeEvent(QCloseEvent* e) override;

private:

	/*
	* owl::NewThreadDlg::NewThreadDlg
	*
	* Private constructor used internally by the delegated constructors
	*
	* @param itemParent Pointer to the Thread or Forum object to
	*		which the post will be created. If null then no action will be taken
	*		after it is submitted (for testing purposes, mostly)
	* @param postText (optional) Any text to pre-populate the text box
	* @param parent (optional) Pointer to the parent of the dialog
	*/
	NewThreadDlg(BoardItemPtr itemParent, NewItemType type, QWidget* parent = nullptr);


    void lockForm();
    void unlockForm();

    void encloseText(QString tag);

    NewItemType     _type;
    BoardItemPtr    _itemParent;
    ParserBasePtr   _parser;

    bool _bSubmitted = false;

    BoardWeakPtr    _board; // weak pointer to the board!

private Q_SLOTS:
    void onSubmitClicked();
    void showError(OwlExceptionPtr);
};

} // namespace
