#pragma once

#include <memory>

#include <QQuickView>
#include <QQuickWidget>
#include <QQuickItem>
#include <QQmlContext>

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

class NewConnectionQuickWidget : public QQuickWidget
{
	Q_OBJECT

public:
    NewConnectionQuickWidget(QWidget* parent = nullptr, const QUrl& qmlfile = QUrl{})
        : QQuickWidget(parent)
    {
        setFocusPolicy(Qt::TabFocus);
        setResizeMode(QQuickWidget::SizeRootObjectToView);

        QQmlContext* root = rootContext();
        root = this->rootContext();
        root->setContextProperty("newConnectionPage", this);

        setSource(qmlfile);
    }

    Q_INVOKABLE void onOptionSelected(int option)
    {
        this->newConnectionEvent(option);
        // // std::cout << "Option selected: " << option << std::endl;
        // switch (option)
        // {
        //     default:
        //     break;

        //     case 1:
        //     {
        //         this->setSource(QUrl(QStringLiteral("qrc:/NewChatConnection.qml")));
        //         break;
        //     }
        // }
    }

Q_SIGNALS:
    void newConnectionEvent(int option);
};

class NewConnectionDlg : public QDialog
{
	Q_OBJECT

public:
	NewConnectionDlg(QWidget *parent = 0)
        : QDialog(parent), _qmlWidget(new NewConnectionQuickWidget(this, QUrl("qrc:/qml/NewConnectionDlg.qml")))
	{
		this->resize(850, 525);
        _layout = new QHBoxLayout(this);
        _layout->addWidget(_qmlWidget);

        // auto rootObject = _qmlWidget->rootObject();
        QObject::connect(_qmlWidget, SIGNAL(newConnectionEvent(int)), this, SLOT(accept2(int)));
	}


	~NewConnectionDlg() = default;

protected Q_SLOTS:
    void accept2(int selection)
    {
        NewConnectionQuickWidget* widget = nullptr;
        switch (selection)
        {
            default:
            break;

            case 1:
                widget = new NewConnectionQuickWidget(this, QUrl("qrc:/qml/NewChatConnection.qml"));
            break;

            case 2:
                widget = new NewConnectionQuickWidget(this, QUrl("qrc:/qml/NewMessageBoardConnection.qml"));
            break;

            case 3:
                widget = new NewConnectionQuickWidget(this, QUrl("qrc:/qml/NewRedditConnection.qml"));
            break;

            case 4:
            {
                qDebug() << "New Browser Dialog";
                break;
            }
        }

        if (nullptr != widget)
        {
            _qmlWidget->deleteLater();
            _qmlWidget = widget;
            _layout->addWidget(_qmlWidget);
        }
    }


private:
    QQuickWidget* _qmlWidget;
    QHBoxLayout* _layout;
};

} //namespace owl


