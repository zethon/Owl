#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QWebEngineView>

#include "ConnectionFrame.h"

namespace owl
{

class OwlWebBrowser : public owl::ConnectionFrame
{
    Q_OBJECT

public:
    explicit OwlWebBrowser(const std::string& uuid, QWidget *parent = nullptr);

Q_SIGNALS:

private:
    void connectSignals();
    void doGo();

    QLineEdit*      _urlEdit;
    QWebEngineView* _webView;
};

} // namespace owl
