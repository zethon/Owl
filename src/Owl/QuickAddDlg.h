#pragma once

#include <memory>

#include <QQuickView>
#include <QQuickWidget>

#include "ui_QuickAddDlg.h"

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

class NewConnectionDlg : public QDialog
{
	Q_OBJECT

public:
	NewConnectionDlg(QWidget *parent = 0)
		: QDialog(parent)
	{
		this->resize(850, 525);
        auto horizontalLayout = new QHBoxLayout(this);
        auto widget = new QQuickWidget(this);
        horizontalLayout->addWidget(widget);

		widget->setFocusPolicy(Qt::TabFocus);
    	widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
		widget->setSource(QUrl("qrc:/qml/NewConnectionDlg.qml"));
	}

	~NewConnectionDlg() = default;
};

} //namespace owl


