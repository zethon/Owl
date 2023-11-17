#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QWebEngineView>

namespace owl
{

class OwlWebBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit OwlWebBrowser(QWidget *parent = nullptr);

Q_SIGNALS:

private:
    void connectSignals();
    void doGo();

    QLineEdit*      _urlEdit;
    QWebEngineView* _webView;
};

} // namespace owl
