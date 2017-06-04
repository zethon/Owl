#pragma once
#include <QQuickWidget>
#include <QWebEngineView>
#include <QWebChannel>
#include <Parsers/Forum.h>
#include <Parsers/BBCodeParser.h>
#include <Utils/DateTimeParser.h>

namespace owl
{

class PostListWebView;

class PostListWebPage : public QWebEnginePage
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER

public:
    PostListWebPage(QObject* parent = nullptr)
        : QWebEnginePage(parent)
    {
    }

    virtual bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override;
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID) override;
};

class SharedPostObject : public QObject
{
    Q_OBJECT

public:
    SharedPostObject(QObject *parent = 0);
    virtual ~SharedPostObject() = default;

public Q_SLOTS:
    void doQuotePost(uint index);
    void doReplyPost(uint index);
    void doViewImage(const QString &base64, const QString& url);

private:
    PostListWebView* _view = nullptr;
};

class PostListWebView : public QWebEngineView
{
    Q_OBJECT

public:
    PostListWebView(QWidget* parent);
    virtual ~PostListWebView();

    void showPosts(const ThreadPtr thread);
    void resetView(); // clears the post list and sets an empty page
    void clear(); // clears the post list
    void reloadView(); // resets view with current posts and updated settings

    void expandAll();
    void collapseAll();

    ThreadPtr getCurrentThread() const { return _currentThread; }

Q_SIGNALS:
    void quotePost(ThreadPtr, uint);
    void replyPost(ThreadPtr, uint);
    void showImageFromString(const QString& base64);
    void showImageFromArray(const QByteArray& base64);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *e) override;

private:
    PostListWebPage _page;
    owl::BBRegExParser _bbcodeparser;
    ThreadPtr _currentThread;

    QWebChannel* _channelPtr;
    QWebChannel _channel; // who knows, this may need to be dynamic or some crap
    SharedPostObject  _testobj;

    QString _postPageHeader;
    QString _postPageFooter;
    QString _postBit;
    QString _jQuery;

    const QString scrollFirstUnread = "var es=document.getElementById('firstUnread');if(es){es.scrollIntoView({behavior: \"smooth\"});}";
    const QString scrollFirstPost = "var es=document.getElementById('firstPost');if(es){es.scrollIntoView({behavior: \"smooth\"});}";
    const QString scrollLastPost = "var es=document.getElementById('lastPost');if(es){es.scrollIntoView({behavior: \"smooth\"});}";

    DateTimeFormatOptions       _dtOptions;
};

} // namespace
