#include <QVBoxLayout>
#include <QFrame>
#include <QWebEngineView>
#include <QSpacerItem>
#include <QStyle>

#include "WebBrowser.h"

namespace owl
{

OwlWebBrowser::OwlWebBrowser(QWidget *parent)
    : QWidget{parent}
{
    this->setStyleSheet("QWidget { background: white; }");
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(1, 1, 1, 1);

    auto addressFrame = new QFrame(this);
    addressFrame->setMaximumSize(QSize(16777215, 50));
    addressFrame->setFrameShape(QFrame::NoFrame);
    auto horizontalLayout = new QHBoxLayout(addressFrame);
    horizontalLayout->setSpacing(0);
    horizontalLayout->setContentsMargins(5, 0, 5, 0);
    _urlEdit = new QLineEdit(addressFrame);
    _urlEdit->setAttribute(Qt::WA_MacShowFocusRect,0);
    _urlEdit->setStyleSheet("QLineEdit{ border-radius: 5px; }");
    _urlEdit->setPlaceholderText("Search or enter address");
    _urlEdit->setMinimumSize(QSize(0, 30));

    QFont font{_urlEdit->font()};
    font.setPointSize(16);
    _urlEdit->setFont(font);

    horizontalLayout->addWidget(_urlEdit);
    // horizontalLayout->addSpacing(20);

    mainLayout->addWidget(addressFrame);

    auto line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line);

    auto browserFrame = new QFrame(this);
    browserFrame->setFrameShape(QFrame::NoFrame);
    auto verticalLayout = new QVBoxLayout(browserFrame);
    _webView = new QWebEngineView(browserFrame);
    _webView->setUrl(QUrl(QString::fromUtf8("https://owlclient.com/")));
    verticalLayout->addWidget(_webView);
    verticalLayout->setSpacing(0);
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(browserFrame);

    this->setLayout(mainLayout);

    connectSignals();
}

void OwlWebBrowser::connectSignals()
{
    QObject::connect(_urlEdit, &QLineEdit::returnPressed, this, [this]{ doGo(); });
    QObject::connect(_webView, &QWebEngineView::loadFinished, this,
        [this](bool loaded)
        {
            if (!loaded) return;
            _urlEdit->setText("");
            _urlEdit->setPlaceholderText(_webView->page()->title());
            _webView->setFocus();
        });
}

void OwlWebBrowser::doGo()
{
    QUrl url{ this->_urlEdit->text() };
    if (url.scheme().isEmpty()) url.setScheme("https");
    _webView->load(url);
}

} // namespace owl
