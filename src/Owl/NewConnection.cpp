#include <QHBoxLayout>
// #include <QQuickWidget>
// #include <QQuickItem>
#include <QQmlContext>

#include "NewConnection.h"

namespace owl
{

NewConnectionQuickWidget::NewConnectionQuickWidget(QWidget* parent)
    : QQuickWidget(parent)
{
    setFocusPolicy(Qt::TabFocus);
    setResizeMode(QQuickWidget::SizeRootObjectToView);        
    
    QQmlContext* root = rootContext();
    root = this->rootContext();
    root->setContextProperty("newConnectionPage", this);
}

NewConnectionQuickWidget::NewConnectionQuickWidget(QWidget* parent, const QUrl& qmlfile)
    : NewConnectionQuickWidget(parent)
{
    setSource(qmlfile);
}

NewRedditConnectionQuickWidget::NewRedditConnectionQuickWidget(QWidget* parent)
    : NewConnectionQuickWidget(parent)
{
    auto root = rootContext();
    root->setContextProperty("thisPage", this);

    // we have to defer setting the source until the context is set
    setSource(QUrl(QML_FILE));
}

void NewRedditConnectionQuickWidget::onAccept()
{
    qDebug() << "onAccept";
    qDebug() << "redditId: " << _redditId;
    qDebug() << "randomString: " << _randomString;
    qDebug() << "redirectUrl: " << _redirectUrl;
    qDebug() << "scope: " << _scope;
    qDebug() << "userAgent: " << _userAgent;
    qDebug() << "/onAccept";
}

NewConnectionDlg::NewConnectionDlg(QWidget *parent)
    : QDialog(parent),
      _qmlWidget{new NewConnectionQuickWidget(this, QUrl("qrc:/qml/NewConnectionDlg.qml"))}
{
    this->resize(850, 525);
    _layout = new QHBoxLayout(this);
    _layout->setSpacing(0);
    _layout->addWidget(_qmlWidget);

    setupSignals();
}

void NewConnectionDlg::setupSignals()
{
	QObject::connect(_qmlWidget, SIGNAL(newConnectionEvent(int)), this, SLOT(onSelected(int)));
	
	// QObject::connect(_qmlWidget, SIGNAL(onCancelEvent()), this, SLOT(onCancelHandler()));
	QObject::connect(_qmlWidget, SIGNAL(onCancelEvent()), this, SLOT(onCancelHandler()));
}

void NewConnectionDlg::onSelected(int selection)
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
                // widget = new NewConnectionQuickWidget(this, QUrl("qrc:/qml/NewRedditConnection.qml"));
				widget = new NewRedditConnectionQuickWidget(this);

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
            _qmlWidget->setFocus();
			setupSignals();
        }
    }

void NewConnectionDlg::onCancelHandler()
{
	reject();
}

} // namespace owl
