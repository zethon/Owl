#include <QQmlContext>
#include <QQuickItem>
#include <QMenu>
#include <QFileDialog>
#include <QWebEngineSettings>
#include <QNetworkReply>
#include <Utils/Settings.h>
#include <Utils/OwlUtils.h>
#include "Data/Board.h"
#include "PostListWidget.h"

#include  <Utils/OwlLogger.h>

namespace owl
{

PostListWebView::PostListWebView(QWidget* parent)
    : QWebEngineView(parent),
      _testobj(this)
{
    SettingsObject appsetttings;
    _dtOptions.useDefault = appsetttings.read("datetime.format").toString() == "default";
    _dtOptions.usePretty = appsetttings.read("datetime.date.pretty").toBool();
    _dtOptions.dateFormat = appsetttings.read("datetime.date.format").toString();
    _dtOptions.timeFormat = appsetttings.read("datetime.time.format").toString();

    settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
    settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, false);
    settings()->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);
    settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);

    // disable reload
    pageAction(QWebEnginePage::Reload)->disconnect();
    setPage(&_page);

    _page.setWebChannel(&_channel);
    _channel.registerObject(QStringLiteral("postlist"), &_testobj);

    _postPageHeader = owl::getResourceHtmlFile("postPageHeader.html");
    _postPageFooter = owl::getResourceHtmlFile("postPageFooter.html");
    _postBit = owl::getResourceHtmlFile("postPagePostBit.html");

    QFile file;
    file.setFileName(":/js/jquery.min.js");
    file.open(QIODevice::ReadOnly);
    _jQuery = file.readAll();
    _jQuery.append("\nvar qt = { 'jQuery': jQuery.noConflict(true) };");
    file.close();

    QObject::connect(this, &QWebEngineView::loadFinished,
        [this](bool)
        {
            this->page()->runJavaScript(_jQuery);

            // scroll to the first post if necessary
            this->page()->runJavaScript(scrollFirstUnread);
        });
}

PostListWebView::~PostListWebView()
{
    clear();
}

void PostListWebView::showPosts(const ThreadPtr thread)
{
    BoardPtr board = thread->getBoard().lock();
    if (!board)
    {
        OWL_THROW_EXCEPTION(OwlException("Board object is null"));
    }

    _currentThread = thread;
    _bbcodeparser.resetQuoteStyle();
    QString html(_postPageHeader);
    if (SettingsObject().read("postlist.highlight.enabled").toBool())
    {
        html.replace(QStringLiteral("%HIGHLIGHTCOLOR%"), SettingsObject().read("postlist.highlight.color").toString());
    }
    else
    {
        html.replace(QStringLiteral("%HIGHLIGHTCOLOR%"), "transparent");
    }

    const bool showImages = board->getOptions()->getBool("showImages", false);
    settings()->setAttribute(QWebEngineSettings::AutoLoadImages, showImages);

    QString postText;
    auto iCount = 0u;
    bool bExpandPost = false;
    const PostPtr firstUnread = thread->getFirstUnread().lock();
    const auto postNumStart = ((thread->getPageNumber() - 1) * thread->getPerPage()) + 1;

    for (const auto& post : thread->getPosts())
    {
        postText = _postBit;
        postText.replace("{$postid}", post->getId());
        postText.replace("{$postindex}", QString::number(iCount));
        postText.replace("{$username}", post->getAuthor());
        postText.replace("{$dateline}", post->getPrettyTimestamp(_dtOptions));
        postText.replace("{$postnum}", QString::number(postNumStart + iCount));
        postText.replace("{$quoteBtnName}", QString("button%1").arg(iCount));
        postText.replace("{$usericon}", post->getIconUrl().size() > 0 ? post->getIconUrl() : "qrc:/icons/no-avatar.png");

        // TODO: obviously a hack, need to figure out what I was thinking here
        if (board->getParser()->getName().contains("tapatalk", Qt::CaseInsensitive))
        {
            postText.replace("{$posttext}", _bbcodeparser.toHtml(post->getText()));
        }
        else
        {
           postText.replace("{$posttext}", post->getText());
        }

        // if the user just posted then it is possible for firstUnread to be set
        // but for thread->hasUnread() to be false
        if (firstUnread && thread->hasUnread())
        {
            if (firstUnread == post)
            {
                bExpandPost = true;
                postText.replace("{$unreadAnchor}", "firstUnread");
            }
            else
            {
                postText.replace("{$unreadAnchor}", "");
            }
        }
        else
        {
            bExpandPost = true;
            postText.replace("{$unreadAnchor}", "");
        }

        if (bExpandPost)
        {
            postText.replace("{$unreadStyle}", QString());
            postText.replace("{$unreadClass}", "postheader_expanded");
            postText.replace("{$collapseButtonClass}", "collapsebutton_expanded");
        }
        else
        {
            postText.replace("{$unreadStyle}", "none");
            postText.replace("{$unreadClass}", "postheader_collapsed");
            postText.replace("{$collapseButtonClass}", "collapsebutton_collapsed");
        }

        html.append(postText);
        iCount++;
    }

    html.append(_postPageFooter);
    setHtml(html, QUrl(board->getParser()->getBaseUrl()));
}

void PostListWebView::resetView()
{
    clear();
    setUrl(QUrl("qrc:/html/emptyPostList.html"));
}

void PostListWebView::clear()
{
    _currentThread.reset();
}

void PostListWebView::reloadView()
{
    SettingsObject settings;
    _dtOptions.useDefault = settings.read("datetime.format").toString() == "default";
    _dtOptions.usePretty = settings.read("datetime.date.pretty").toBool();
    _dtOptions.dateFormat = settings.read("datetime.date.format").toString();
    _dtOptions.timeFormat = settings.read("datetime.time.format").toString();

    if (_currentThread)
    {
        showPosts(_currentThread);
    }
}

void PostListWebView::expandAll()
{
    page()->runJavaScript("expandAll();");
}

void PostListWebView::collapseAll()
{
    page()->runJavaScript("collapseAll();");
}

void PostListWebView::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu(this);

    QAction* savePageAction = nullptr;
    auto savePageImpl = [this]()
    {
        const QString filename = QFileDialog::getSaveFileName();
        if (!filename.isNull())
        {
            page()->toHtml([this,filename](const QString& html)
            {
                if (QFile::exists(filename))
                {
                    QFile::remove(filename);
                }

                QFile outfile(filename);
                outfile.open(QIODevice::WriteOnly | QIODevice::Text);
                QTextStream stream(&outfile);
                stream << html;
                outfile.close();
            });
        }
    };

#ifdef QT_DEBUG
    QAction* copySrc = menu.addAction("Copy Source");
    QObject::connect(copySrc, &QAction::triggered, [this]()
    {
        this->page()->toHtml([this](const QString& html) mutable
        {
            qApp->clipboard()->setText(html);
        });
    });

    savePageAction = menu.addAction(tr("Save Page As..."), savePageImpl);
    menu.addSeparator();
#endif

    if (_currentThread && _currentThread->getPosts().size() > 0)
    {
        menu.addAction(tr("Copy"),[this]()
        {//    if (level == JavaScriptConsoleMessageLevel::WarningMessageLevel)

            this->triggerPageAction(QWebEnginePage::Copy);
        });

        if (!savePageAction)
        {
            savePageAction = menu.addAction(tr("Save Page As..."), savePageImpl);
        }
    }

    menu.exec(e->globalPos());
}

SharedPostObject::SharedPostObject(QObject *parent)
    : QObject(parent)
{
    _view = static_cast<PostListWebView*>(parent);
    Q_ASSERT(_view);
}

void SharedPostObject::doQuotePost(uint index)
{
    auto thread = _view->getCurrentThread();
    _view->quotePost(thread, index);
}

void SharedPostObject::doReplyPost(uint index)
{
    auto thread = _view->getCurrentThread();
    _view->replyPost(thread, index);
}

void SharedPostObject::doViewImage(const QString &base64, const QString& url)
{
    // base64 will be the base64 text IF it's able to be retrieved, otherwise
    // we will make a network call to the url of the time and try that
    if (base64.size() > 0)
    {
        Q_EMIT _view->showImageFromString(base64);
    }
    else if (url.size() > 0)
    {
        QNetworkAccessManager *manager = new QNetworkAccessManager();
        QObject::connect(manager, &QNetworkAccessManager::finished,
            [this](QNetworkReply* reply)
            {
                auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                if (status == 200)
                {
                    const auto buffer = reply->readAll();
                    Q_EMIT _view->showImageFromArray(buffer);
                }
            });

        manager->get(QNetworkRequest(QUrl(url)));
    }
}

bool PostListWebPage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool)
{
    if (type == QWebEnginePage::NavigationTypeLinkClicked)
    {
        QDesktopServices::openUrl(url);
        return false;
    }

    return true;
}

void PostListWebPage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID)
{
    auto loggerLevel = spdlog::level::info;
    if (level == JavaScriptConsoleMessageLevel::WarningMessageLevel)
    {
        loggerLevel = spdlog::level::warn;
    }
    else if (level == JavaScriptConsoleMessageLevel::ErrorMessageLevel)
    {
        loggerLevel = spdlog::level::err;
    }

    owl::rootLogger()->log(loggerLevel,
        "({}:{}): {}", lineNumber, message.toStdString(), sourceID.toStdString());
}

}
