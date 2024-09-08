#pragma once

#include <QQuickWidget>
#include <QDialog>

namespace owl
{
class NewConnectionQuickWidget : public QQuickWidget
{
	Q_OBJECT

public:
    NewConnectionQuickWidget(QWidget* parent);
    NewConnectionQuickWidget(QWidget* parent, const QUrl& qmlfile);
    ~NewConnectionQuickWidget() = default;

    Q_INVOKABLE void onOptionSelected(int option)
    {
        this->newConnectionEvent(option);
    }

    Q_INVOKABLE void onCancel()
    {
        this->onCancelEvent();
    }

Q_SIGNALS:
    void newConnectionEvent(int option);
    void onCancelEvent();
};


class NewRedditConnectionQuickWidget : public NewConnectionQuickWidget
{
    Q_OBJECT

    constexpr static const char* QML_FILE = "qrc:/qml/NewRedditConnection.qml";
   
    Q_PROPERTY(QString redditId READ redditId WRITE setRedditId NOTIFY redditIdChanged)
    Q_PROPERTY(QString randomString READ randomString WRITE setRandomString NOTIFY randomStringChanged)
    Q_PROPERTY(QString redirectUrl READ redirectUrl WRITE setRedirectUrl NOTIFY redirectUrlChanged)
    Q_PROPERTY(QString scope READ scope WRITE setScope NOTIFY scopeChanged)
    Q_PROPERTY(QString userAgent READ userAgent WRITE setUserAgent NOTIFY userAgentChanged)

public:
    NewRedditConnectionQuickWidget(QWidget* parent);

    Q_INVOKABLE void onAccept();

    QString redditId() const { return _redditId; }
    void setRedditId(const QString& var) { _redditId = var; }
    QString randomString() const { return _randomString; }
    void setRandomString(const QString& var) { _randomString = var; }
    QString redirectUrl() const { return _redirectUrl; }
    void setRedirectUrl(const QString& var) { _redirectUrl = var; }
    QString scope() const { return _scope; }
    void setScope(const QString& var) { _scope = var; }
    QString userAgent() const { return _userAgent; }
    void setUserAgent(const QString& var) { _userAgent = var; }

Q_SIGNALS:
    void redditIdChanged(const QString& var);
    void randomStringChanged(const QString& var);
    void redirectUrlChanged(const QString& var);
    void scopeChanged(const QString& var);
    void userAgentChanged(const QString& var);

private:
    QString _redditId;
    QString _randomString;
    QString _redirectUrl;
    QString _scope;
    QString _userAgent;
};

class NewConnectionDlg : public QDialog
{
    Q_OBJECT

public:
    NewConnectionDlg(QWidget *parent = 0);
    ~NewConnectionDlg() = default;

protected Q_SLOTS:
    void onSelected(int selection);
    void onCancelHandler();

private:
    void setupSignals();

    QQuickWidget* _qmlWidget;
    QHBoxLayout* _layout;
};
} // namespace owl
