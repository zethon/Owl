// THE GOAL WITH THIS FILE IS TO DELETE IT
#pragma once

#include <memory>

#include <QQuickView>
#include <QQuickWidget>
#include <QQuickItem>
#include <QQmlContext>

#include "ui_QuickAddDlg.h"

#include "Utils/OwlUtils.h"

namespace Ui
{
	class QuickAddDlg;
}

namespace owl
{

class Board;
using BoardPtr = std::shared_ptr<Board>;

class ConfiguringBoardDlg;

class QuickAddDlg : public QDialog, public Ui::QuickAddDlg
{
	Q_OBJECT
	
public:
	~QuickAddDlg() = default;

	QuickAddDlg(QWidget *parent = 0);
	
	void setParserByName(const QString& name);

protected Q_SLOTS:
	void onConfigureFinished(int result);

Q_SIGNALS:
	void newBoardAddedEvent(BoardPtr);

protected:
	virtual void accept();
    
private:
	ConfiguringBoardDlg* _configureDlg;
};

// class NewConnectionDlg : public QQuickView
// {
// 	Q_OBJECT

// public:
// 	NewConnectionDlg(QWindow *parent = 0)
// 		: QQuickView(parent)
// 	{
// 		this->setObjectName("NewConnectionDlg");
// 		this->setSource(QUrl("qrc:/qml/NewConnectionDlg.qml"));
// 	}

// 	~NewConnectionDlg() = default;
// };


} //namespace owl


