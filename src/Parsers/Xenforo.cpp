// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#include "../Utils/OwlUtils.h"
#include "../Utils/QSgml.h"
#include "../Utils/QSgmlTag.h"
#include "../Utils/WebClient.h"
#include "Xenforo.h"
#include <cmath>

#include <spdlog/spdlog.h>

namespace owl
{

Q_INVOKABLE Xenforo::Xenforo(const QString &baseUrl)
    : ParserBase(XENFORO_NAME,XENFORO_PRETTYNAME, baseUrl),
      _logger { spdlog::get("Owl")->clone("Xenforo") }
{
    addWatcher(&_webclient);
}

ParserBasePtr Xenforo::clone(ParserBasePtr other)
{
    XenforoPtr retval = std::make_shared<Xenforo>(getBaseUrl());
    ParserBase::clone(retval);

    retval->_webclient.setCurlHandle(_webclient.getCurlHandle());
    retval->_logoutUrl = _logoutUrl;

    return retval;
}

bool Xenforo::canParse(const QString& data)
{
    bool retval = false;

    QSgml doc;
    if (doc.parse(data))
    {
        const auto htmlNode = doc.getElementsByName("html");
        retval = htmlNode.size() > 0 && htmlNode.at(0)->getArgValue("id") == "XenForo";
    }

    return retval;
}

QString Xenforo::getPostQuote(PostPtr post)
{
    QString retval;

    const QString quoteUrl = QString("%1/%2/reply&quote=%3")
        .arg(this->getBaseUrl())
        .arg(post->getParent()->getId())
        .arg(post->getId());

    QString data;

    try
    {
        data = _webclient.DownloadString(quoteUrl);

        QSgml doc;

        if (doc.parse(data))
        {
            const auto textareanode = doc.DocTag->getFirstElementByName("textarea", "name", QRegExp{"^message$"});
            if (textareanode)
            {
                retval = QString("%1\n\n").arg(doc.getText(textareanode));
            }
        }
    }
    catch (const WebException&)
    {
        // some boards do not use this quoting behavior and force an inline/AJAX behvior
        // in which case we can't get the proper quote
    }

    if (retval.isEmpty())
    {
        retval = QString("[QUOTE=\"%1, post: %2, member: %3\"]%4[/QUOTE]\n\n")
            .arg(post->getAuthor())
            .arg(post->getId())
            .arg(post->getAuthor())
            .arg(post->getText().trimmed());
    }

    return retval;
}

QVariant Xenforo::doGetUnreadForums()
{
    ForumList retval;
    const QString url = QString("%1/find-new/posts")
        .arg(this->getBaseUrl());

    const QString data = _webclient.DownloadString(url, WebClient::NOCACHE);

    QSgml doc;
    if (doc.parse(data))
    {
        for (QSgmlTag* linode : doc.DocTag->getElementsByName("li", "class", QRegExp{"unread"}))
        {
            const auto forumlinknode = linode->getFirstElementByName("a", "class", QRegExp{"forumLink"});
            if (forumlinknode)
            {
                const auto forumId = forumlinknode->getArgValue("href");
                const auto forumName = doc.getText(forumlinknode);

                if (!forumId.isEmpty() && !forumName.isEmpty() &&
                    std::find_if(retval.begin(), retval.end(),
                        [&forumId, &forumName](const ForumPtr f)
                        {
                            return f->getId() == forumId && f->getName() == forumName;
                        }) == retval.end())
                {
                    auto newforum = std::make_shared<Forum>(forumId);
                    newforum->setName(forumName);
                    newforum->setHasUnread(true);
                    retval.push_back(newforum);
                }
            }
        }
    }

    return QVariant::fromValue(retval);
}

QVariant Xenforo::doMarkForumRead(ForumPtr forumInfo)
{
    const QString url = QString("%1/forums/-/mark-read")
        .arg(this->getBaseUrl());

    const QString predata = _webclient.DownloadString(url);

    QSgml doc;
    if (doc.parse(predata))
    {
        const auto datenode = doc.DocTag->getFirstElementByName("input", "name", QRegExp{"^date$"});
        const auto tokennode = doc.DocTag->getFirstElementByName("input", "name", QRegExp{"^_xfToken$"});

        if (datenode && tokennode)
        {
            StringMap params;

            QString forumId { "0" };
            if (!forumInfo->IsRoot())
            {
                QRegularExpression idRegex { "\\.(?<id>\\d+)/*$" };
                auto match = idRegex.match(forumInfo->getId());
                if (match.hasMatch())
                {
                    forumId = match.captured("id");
                }
            }

            params.add("node_id", forumId);
            params.add("date", datenode->getArgValue("value"));
            params.add("_xfToken", tokennode->getArgValue("value"));
            params.add("_xfConfirm", "1");

            _webclient.UploadString(url, params.encode());
        }
    }

    return QVariant::fromValue(forumInfo);
}

QVariant Xenforo::doLogin(const LoginInfo& loginInfo)
{
    owl::StringMap result;
    result.setOrAdd("success", (bool)false); // assume failure!

    StringMap params;

    // set up our login-info
    params.add("login", loginInfo.first);
    params.add("password", loginInfo.second);

    // and other params we want to pass
    params.add("cookie_check", "0");
    params.add("remember", "1");
    params.add("_xfToken", QString());

    const QString loginUrl { this->getBaseUrl() + "/login/login" };

    // The problems related to the session cookie getting rest seemed to be related to passing cookies
    // at the time of login. Once it was discovered that this fixed a long standing issue in Tapatalk
    // (Bug #117) the same fix was tried here and it worked! Thus removing the need for a hack in the
    // getBoardwareInfo() method below.
//    _webclient.getCookieJar()->deleteAllCookies();
    _webclient.deleteAllCookies();

    QString data = _webclient.UploadString(loginUrl, params.encode());


    QSgml parseDoc;
    if (parseDoc.parse(data))
    {
        // NOTE: To confirm a successful login we look for a <a class="LogOut"> element that provides
        // us with the hash we want in order to logout, which is why we save the link in a class
        // variable.
        const auto logoutnodes = parseDoc.getElementsByName("a", "class", QRegExp{"LogOut"});

        if (logoutnodes.size() > 0)
        {
            _logoutUrl = logoutnodes.at(0)->getArgValue("href");

            // signify success
            result.setOrAdd("success", (bool)true);
            result.add("boardware", "xenforo");
        }
    }
    else
    {
        _logger->warn("Could not parse login response");
    }

    return QVariant::fromValue(result);
}

QVariant Xenforo::doLogout()
{
    StringMap result;

    if (!_logoutUrl.isEmpty())
    {
        _webclient.DownloadString(_logoutUrl, WebClient::NOCACHE);
        _logoutUrl.clear();
        result.add("success",(bool)true);
    }
    else
    {
        result.add("success",(bool)false);
    }

    return QVariant::fromValue(result);
}

QVariant Xenforo::doGetBoardwareInfo()
{
    StringMap result;

    result.setOrAdd("success", (bool) false); // assume failure!

    const QString data = _webclient.DownloadString(this->getBaseUrl());

    QSgml doc;
    if (doc.parse(data))
    {
        const auto htmlNode = doc.getElementsByName("html");
        if (htmlNode.size() > 0 && htmlNode.at(0)->getArgValue("id") == "XenForo")
        {
            const auto titleNode = doc.getElementsByName("title");
            if (titleNode.size() > 0)
            {
                result.setOrAdd("success", (bool)true);
                result.setOrAdd("name", doc.getText(titleNode.at(0)));
                result.setOrAdd("version", (QString)"1.x");
                result.add("boardware", "xenforo");
            }
        }
    }

    return QVariant::fromValue(result);
}

QVariant Xenforo::doGetForumList(const QString &forumId)
{
    ForumList retval;

    if (forumId == this->getRootForumId())
    {
        retval = this->getRootForumPrivate();
    }
    else
    {
        retval = this->getForumsPrivate(forumId);
    }

    return QVariant::fromValue(retval);
}

QVariant Xenforo::doThreadList(ForumPtr forumInfo, int options)
{
    ThreadList retval;

    QString url;

    if (forumInfo->getId().endsWith("/"))
    {
        url = QString("%1/%2page-%3")
            .arg(this->getBaseUrl())
            .arg(forumInfo->getId())
            .arg(forumInfo->getPageNumber());
    }
    else
    {
        url = QString("%1/%2/page-%3")
            .arg(this->getBaseUrl())
            .arg(forumInfo->getId())
            .arg(forumInfo->getPageNumber());
    }

    if (url.endsWith("page-1"))
    {
        url.replace(QRegExp{"page\\-1$"}, QString());
    }

    uint requestOptions = 0;
    if (options & ParserEnums::REQUEST_NOCACHE)
    {
        requestOptions |= WebClient::NOCACHE;
    }

    const QString data = _webclient.DownloadString(url);

    QSgml doc;
    if (doc.parse(data))
    {
        const auto listItems = doc.getElementsByName("li", "class", QRegExp{"discussionListItem"});
        for (QSgmlTag* liChild : listItems)
        {
            auto titleInfo = liChild->getFirstElementByName("a", "class", QRegExp{"PreviewTooltip"});
            auto authorInfo = liChild->getFirstElementByName("a", "class", QRegExp{"username"});
            QSgmlTag* lastPostDiv = liChild->getFirstElementByName("div", "class", QRegExp{"lastPost"});

            if (titleInfo && authorInfo && lastPostDiv)
            {
                const auto lastAuthorNode = lastPostDiv->getFirstElementByName("a", "class", QRegExp{"username"});
                if (lastAuthorNode)
                {
                    bool bHasUnread = false;

                    QString strId = titleInfo->getArgValue("href");
                    if (strId.endsWith("/unread"))
                    {
                        bHasUnread = true;
                        strId = strId.replace(QRegExp{"/unread$"}, QString());
                    }

                    ThreadPtr newthread = std::make_shared<Thread>(strId);
                    newthread->setParent(forumInfo);
                    newthread->setTitle(doc.getText(titleInfo));
                    newthread->setAuthor(doc.getText(authorInfo));
                    newthread->setHasUnread(bHasUnread);
                    newthread->setSticky(liChild->getArgValue("class").contains("sticky"));

                    PostPtr lastpost = std::make_shared<Post>("-1");
                    lastpost->setAuthor(doc.getText(lastAuthorNode));

                    // try to get the user's avatar
                    QSgmlTag* avatarEl = liChild->getFirstElementByName("div","class",QRegExp{"posterAvatar"});
                    if (avatarEl)
                    {
                        QSgmlTag* imgEl = avatarEl->getFirstElementByName("img","src",QRegExp{});
                        if (imgEl)
                        {
                            const QString src = imgEl->getArgValue("src");
                            if (src.startsWith("http://") || src.startsWith("https://"))
                            {
                                newthread->setIconUrl(src);
                            }
                            else
                            {
                                const QString iconUrl = QString("%1/%2").arg(getBaseUrl()).arg(imgEl->getArgValue("src"));
                                newthread->setIconUrl(iconUrl);
                            }
                        }
                    }

                    QDateTime dt = owl::parseDateTime(lastPostDiv, &doc);
                    if (dt.isValid())
                    {
                        lastpost->setDatelineString(dt.toString("MM-dd-yyyy hh:mm AP"));
                        lastpost->setDateTime(dt);
                    }
                    else
                    {
                        _logger->warn("Could not extract a timestamp from thread '{}' ({}) at url '{}'",
                            newthread->getTitle().toStdString(), newthread->getId().toStdString(), url.toStdString());
                    }

                    newthread->setLastPost(lastpost);

                    QSgmlTag* subTitle = liChild->getFirstElementByName("h4", "class", QRegExp{"subtitle"});
                    if (subTitle)
                    {
                        newthread->setPreviewText(doc.getText(subTitle));
                    }

                    // get the number of replies
                    const auto repliesnode = liChild->getFirstElementByName("div", "class", QRegExp{"stats"});
                    if (repliesnode)
                    {
                        const auto dlclass = repliesnode->getFirstElementByName("dl", "class", QRegExp {"major"});
                        if (dlclass && dlclass->Children.size() > 1 && dlclass->Children.at(1)->Name == "dd")
                        {
                            bool bok = false;
                            const QString countText = doc.getText(dlclass->Children.at(1)).replace(",", QString());
                            const auto replycount = countText.toUInt(&bok);
                            if (bok)
                            {
                                newthread->setReplyCount(replycount);
                            }
                        }
                    }

                    retval.push_back(newthread);
                }
                else
                {
                    _logger->warn("Could not find any author information for a post at url '{}'", url.toStdString());
                }
            }
            else
            {
                if (!titleInfo)
                {
                    _logger->warn("Could not find any title information for a post at url '{}'", url.toStdString());
                }
                else if (!authorInfo)
                {
                    _logger->warn("Could not find any author information for a post at url '{}'", url.toStdString());
                }
                else if (!lastPostDiv)
                {
                    _logger->warn("Could not find any last post information for a post at url '{}'", url.toStdString());
                }
            }
        }

        const auto pagenav = doc.getElementsByName("div", "class", QRegExp{"PageNav"});
        if (pagenav.size() > 0)
        {
            bool ok;
            const auto pageCount = pagenav.at(0)->getArgValue("data-last").toUInt(&ok);
            if (ok)
            {
                forumInfo->setPageCount(pageCount);
            }
        }
    }
    else
    {
        _logger->warn("The url '%1' could not be parsed", url.toStdString());
    }

    forumInfo->getThreads().clear();
    forumInfo->getThreads().append(retval);
    return QVariant::fromValue(forumInfo);
}

QVariant Xenforo::doGetPostList(ThreadPtr threadInfo, ParserBase::PostListOptions listOption, int webOptions)
{
    PostList retval;

    QString url;

    {
        QString strId = threadInfo->getId();

        if (listOption == PostListOptions::FIRST_UNREAD && !strId.endsWith("/unread"))
        {
            if (strId.endsWith("/"))
            {
                url = QString("%1/%2unread")
                    .arg(this->getBaseUrl())
                    .arg(strId);
            }
            else
            {
                url = QString("%1/%2/unread")
                    .arg(this->getBaseUrl())
                    .arg(strId);
            }
        }
        else if (listOption == PostListOptions::FIRST_POST)
        {
            strId = strId.replace(QRegExp{"/unread"}, QString());

            if (strId.endsWith("/"))
            {
                url = QString("%1/%2page-%3")
                    .arg(this->getBaseUrl())
                    .arg(strId)
                    .arg(threadInfo->getPageNumber());
            }
            else
            {
                url = QString("%1/%2/page-%3")
                    .arg(this->getBaseUrl())
                    .arg(strId)
                    .arg(threadInfo->getPageNumber());
            }
        }
        else if (listOption == PostListOptions::LAST_POST)
        {
            OWL_THROW_EXCEPTION(OwlException("LAST_POST not implemented in XenForo parser"));
        }
    }

    uint requestOptions = 0;
    if (webOptions & ParserEnums::REQUEST_NOCACHE)
    {
        requestOptions |= WebClient::NOCACHE;
    }

    const QString data = _webclient.DownloadString(url);

    QSgml doc;
    if (doc.parse(data))
    {
        // we need to get the postID of the post to which we were redirected
        QString strUnreadId;
        if (listOption == PostListOptions::FIRST_UNREAD)
        {
            QRegularExpression postIdRx { "post\\-(?<id>\\d+)$" };
            auto match = postIdRx.match(_webclient.getLastRequestUrl());
            if (match.hasMatch())
            {
                strUnreadId = match.captured("id");
            }
        }

        const auto pagenav = doc.getElementsByName("div", "class", QRegExp{"PageNav"});
        if (pagenav.size() > 0)
        {
            bool ok;
            auto iTemp = pagenav.at(0)->getArgValue("data-last").toUInt(&ok);
            if (ok)
            {
                threadInfo->setPageCount(iTemp);
            }

            iTemp  = pagenav.at(0)->getArgValue("data-page").toUInt(&ok);
            if (ok)
            {
                threadInfo->setPageNumber(iTemp);
            }
        }

        int index = ((threadInfo->getPageNumber() - 1) * threadInfo->getPerPage()) + 1;

        // <li id="post-1352017"
        const auto linodes = doc.getElementsByName("li", "id", QRegExp{"post\\-\\d+"});
        for (const auto node : linodes)
        {
            // NOTE: XenForo postIDs are formatted like "post-XXX", however this can't
            // be used as Owl's internal postID since the PostListView's HTML generates
            // Javscript that uses the postID as part of an object name. But the - (dash)
            // in the object name is a syntatic error in Javascript (and most languages!)
            // and thus the QUOTE button doesn't work. To fix this, we hack out the
            // "post-" part of the ID and use just the numeral
            const auto strId = node->getArgValue("id").replace("post-", QString());

            const auto strAuthor = node->getArgValue("data-author");

            auto newpost = std::make_shared<Post>(strId);
            newpost->setAuthor(strAuthor);
            newpost->setParent(threadInfo);

            const auto textnode = node->getFirstElementByName("blockquote", "class", QRegExp{"messageText"});
            if (textnode)
            {
                const auto rawtext = extractMessageText(doc.getInnerHtml(textnode));
                newpost->setText(rawtext);

                QDateTime dt = owl::parseDateTime(node, &doc);
                if (dt.isValid())
                {
                    newpost->setDatelineString(dt.toString("MM-dd-yyyy hh:mm AP"));
                    newpost->setDateTime(dt);
                }
                else
                {
                    _logger->warn("Could not extract a timestamp from post id '{}' at url '{}'",
                        strId.toStdString(), url.toStdString());
                }

                // extract the user avatar
                QSgmlTag* avatarEl = node->getFirstElementByName("div","class",QRegExp{"avatarHolder"});
                if (avatarEl)
                {
                    QSgmlTag* imgEl = avatarEl->getFirstElementByName("img","src",QRegExp{});
                    if (imgEl)
                    {
                        const QString src = imgEl->getArgValue("src");
                        if (src.startsWith("http://") || src.startsWith("https://"))
                        {
                            newpost->setIconUrl(src);
                        }
                        else
                        {
                            const QString iconUrl = QString("%1/%2").arg(getBaseUrl()).arg(imgEl->getArgValue("src"));
                            newpost->setIconUrl(iconUrl);
                        }
                    }
                }

                newpost->setIndex(index++);
                retval.push_back(newpost);

                if (listOption == PostListOptions::FIRST_UNREAD && strUnreadId == strId)
                {
                    threadInfo->setFirstUnreadPost(newpost);
                }
            }
        }
    }

    threadInfo->getPosts().clear();
    threadInfo->getPosts().append(retval);

    return QVariant::fromValue(threadInfo);
}

QVariant Xenforo::doSubmitNewThread(ThreadPtr threadInfo)
{
    ThreadPtr retval;

    // need to grab the create thread form to get the _xfToken value to use
    QString strId = { threadInfo->getParent()->getId() };
    strId = strId.replace(QRegExp{"/unread$"}, QString());

    const QString createurl = QString("%1/%2/create-thread")
        .arg(this->getBaseUrl())
        .arg(strId);

    const QString data = _webclient.DownloadString(createurl);

    QSgml createDoc;
    if (createDoc.parse(data))
    {
        const auto tokennode = createDoc.DocTag->getFirstElementByName("input", "name", QRegExp{"_xfToken"});
        if (tokennode && !tokennode->getArgValue("value").isEmpty())
        {
            StringMap params;

            // this looks required
            params.add("_xfToken", tokennode->getArgValue("value"));

            // encode the title and message
            QString s((QString)QUrl::toPercentEncoding(threadInfo->getTitle()));
            s = s.replace("%20","+");
            params.add("title",s);

            s = (QString)QUrl::toPercentEncoding(threadInfo->getPosts().at(0)->getText());
            s = s.replace("%20","+");
            params.add("message",s);

            s = (QString)QUrl::toPercentEncoding((QString)threadInfo->getTags());
            s = s.replace("%20","+");
            params.add("tags",s);

             QString addurl = QString("%1/%2/add-thread")
                .arg(this->getBaseUrl())
                .arg(strId);

            const QString replyStr = _webclient.UploadString(addurl, params.encode());

            QSgml replydoc;

            if (replydoc.parse(replyStr))
            {
                const auto permalinknode = replydoc.DocTag->getFirstElementByName("a", "title", QRegExp{"Permalink"});
                if (permalinknode && !permalinknode->getArgValue("href").isEmpty())
                {
                    retval = std::make_shared<Thread>(permalinknode->getArgValue("href"));
                }
            }
        }
    }

    return QVariant::fromValue(retval);
}

QVariant Xenforo::doSubmitNewPost(PostPtr postInfo)
{
    PostPtr retpost;

    // need to grab the create thread form to get the _xfToken value to use
    QString strId = { postInfo->getParent()->getId() };
    strId = strId.replace(QRegExp{"/unread$"}, QString());

    const QString createurl = QString("%1/%2/reply")
        .arg(this->getBaseUrl())
        .arg(strId);

    const QString data = _webclient.DownloadString(createurl);

    QSgml createDoc;
    if (createDoc.parse(data))
    {
        const auto tokennode = createDoc.DocTag->getFirstElementByName("input", "name", QRegExp{"_xfToken"});
        if (tokennode && !tokennode->getArgValue("value").isEmpty())
        {
            StringMap params;

            // this looks required
            params.add("_xfToken", tokennode->getArgValue("value"));

            QString s = (QString)QUrl::toPercentEncoding(postInfo->getText());
            s = s.replace("%20","+");
            params.add("message",s);

            QString addurl = QString("%1/%2/add-reply")
               .arg(this->getBaseUrl())
               .arg(strId);

            const QString replyStr = _webclient.UploadString(addurl, params.encode());

            QSgml replyDoc;
            if (replyDoc.parse(replyStr))
            {
                // NOTE: There's no real way to discern the postID of the post we just submitted from the rest of the posts
                // on the page. What we do instead is get a is of all the posts on reponse and assume that the last one is
                // the post we just submitted. This should work *most* of the time.
                const QList<QSgmlTag*> linodes = replyDoc.getElementsByName("li", "id", QRegExp{"post\\-\\d+"});
                if (linodes.size() >0)
                {
                    // As noted above, the postID needs to be numeric. It probably isn't too important here but
                    // we'll be consistent.
                    const auto strId2 = linodes.back()->getArgValue("id").replace("post-", QString());
                    retpost = std::make_shared<Post>(strId2);
                    postInfo->getParent()->addChild(retpost);
                }
            }
        }
    }

    return QVariant::fromValue(retpost);
}

//********************************************
// PRIVATE METHODS
//********************************************

ForumList Xenforo::getRootForumPrivate()
{
    ForumList retval;

    const QString url = QString("%1/forums")
        .arg(this->getBaseUrl());

    const QString data = _webclient.DownloadString(url);

    QSgml parseDoc;
    if (!parseDoc.parse(data))
    {
        OWL_THROW_EXCEPTION(OwlException("bad parsing"));
    }

    const auto tags = parseDoc.getElementsByName("li", "class", QRegExp{"level_1"});
    if (tags.size())
    {
        QString strId;
        QString strName;
        ForumPtr newforum;
        auto iDisplayOrder = 1u;

        for (const auto t : tags)
        {
            strId.clear();
            strName.clear();
            newforum.reset();

            // luckily it seems that all of xenforo's 4 types of "nodes" all are labeled with this
            const auto nodeTitle = t->getFirstElementByName("h3", "class", QRegExp{"nodeTitle"});
            if (nodeTitle)
            {
                const auto alink = nodeTitle->getFirstElementByName("a", "href", QRegExp());
                if (alink)
                {
                    strId = alink->getArgValue("href");
                    strName = parseDoc.getText(alink);

                    if (!strId.isEmpty() && !strName.isEmpty())
                    {
                        newforum = std::make_shared<Forum>(strId);
                        newforum->setName(strName);
                        newforum->setDisplayOrder(iDisplayOrder++);
                    }
                }
            }

            if (newforum)
            {
                const QString strClass { t->getArgValue("class") };
                newforum->setForumType(getForumType(strClass));

                if (newforum->getForumType() == Forum::LINK)
                {
                    const QString forumlink = QString("%1/%2")
                        .arg(this->getBaseUrl())
                        .arg(strId);

                    newforum->setVar("link", forumlink);
                }

                retval.push_back(newforum);
            }
        }
    }

    return retval;
}

ForumList Xenforo::getForumsPrivate(const QString &id)
{
    const QRegExp classNode { "node[\\s\\\"]+" };
    ForumList retval;

    // the id could be something like ".#general", but this causes problems in the webclient
    // because the initial request (http://board.com/.#general) will get redirected
    // (to http://board.com/./) which is a 404. If we strip off the '.' before the request
    // is ever made, then we can avoid this
    const QString idFix = id.startsWith(".#") ? id.mid(1): id;

    const QString url = QString("%1/%2")
        .arg(this->getBaseUrl())
        .arg(idFix);

    const QString data = _webclient.DownloadString(url);

    QSgml parseDoc;
    if (!parseDoc.parse(data))
    {
        OWL_THROW_EXCEPTION(OwlException("Could not parse response"));
    }

    if (id.indexOf('#') != -1)
    {
        const auto alink = parseDoc.getElementsByName("a", "href", id);
        if (alink.size() > 0)
        {
            // we have to go 3 nodes up to get the parent <li> element which has a child <ol> element we want
            const auto linode = getXParentsUp(alink[0], 4);
            if (linode)
            {
                // ok, now the <ol class="nodeList"> elementwill have all this forum's children info as
                // direct children
                QSgmlTag* nodelistEl = linode->getFirstElementByName("ol", "class", QRegExp{"nodeList"});
                if (nodelistEl)
                {
                    auto iDisplayOrder = 1u;
                    for (QSgmlTag* node : nodelistEl->Children)
                    {
                        if (node->Name != "li")
                        {
                            continue;
                        }

                        auto h3Element = getXChildrenDown(node, 3);
                        if (h3Element && h3Element->Children.size() > 0)
                        {
                            const auto alink2 = h3Element->Children.at(0);
                            const QString strId = alink2->getArgValue("href");
                            const QString strName = parseDoc.getText(alink2);

                            if (!strId.isEmpty() && !strName.isEmpty())
                            {
                                ForumPtr newforum = std::make_shared<Forum>(strId);
                                newforum->setName(strName);
                                newforum->setDisplayOrder(iDisplayOrder++);

                                const QString strClass = node->getArgValue("class");
                                newforum->setForumType(getForumType(strClass));

                                retval.push_back(newforum);
                            }
                        }
                        else
                        {
                            qDebug() << "No <h3> element found";
                        }
                    }
                }
                else
                {
                    qDebug() << "No <ol class=\"nodeList\"> element could be found";
                }
            }
            else
            {
                qDebug() << "No 4th parent <li class=\"node ...\"> element could be found";
            }
        }
        else
            {
                qDebug() << "No <a href> element with value containing '" << id << "' could be found";
            }
    }
    else
    {
        const QRegExp nodeListRx { "nodeList" };
        const auto nodeList = parseDoc.getElementsByName("ol", "class", nodeListRx);

        // NOTE: no error if the size equal 0 because that is indicative of there being no subforums
        if (nodeList.size() > 0)
        {
            if (nodeList.size() > 1)
            {
                _logger->warn("More than one <ol class=\"nodeList\"> found, using the first one");
            }

            auto iDisplayOrder = 1u;
            for (auto const node : nodeList.at(0)->Children)
            {
                if (node->Name == "li")
                {
                    auto h3Element = getXChildrenDown(node, 3);
                    if (h3Element && h3Element->Children.size() > 0)
                    {
                        const auto alink = h3Element->Children.at(0);
                        const QString strId = alink->getArgValue("href");
                        const QString strName = parseDoc.getText(alink);

                        if (!strId.isEmpty() && !strName.isEmpty())
                        {
                            ForumPtr newforum = std::make_shared<Forum>(strId);
                            newforum->setName(strName);
                            newforum->setDisplayOrder(iDisplayOrder++);

                            const QString strClass = node->getArgValue("class");
                            newforum->setForumType(getForumType(strClass));

                            if (newforum->getForumType() == Forum::LINK)
                            {
                                const QString forumlink = QString("%1/%2")
                                    .arg(this->getBaseUrl())
                                    .arg(strId);

                                newforum->setVar("link", forumlink);
                            }

                            retval.push_back(newforum);
                        }
                    }
                    else
                    {
                        _logger->warn("No <h3> element could be found. Skipping potential forum.");
                    }
                }
            }
        }
    }

    return retval;
}

//********************************************
// GLOBAL METHODS
//********************************************

QSgmlTag* getXParentsUp(QSgmlTag* source, uint iUp)
{
    QSgmlTag* retval = nullptr;
    QSgmlTag* temp = source;

    for (auto i = 0u; i < iUp; i++)
    {
        if (temp)
        {
            temp = temp->Parent;
        }
        else
        {
            break;
        }
    }

    if (temp)
    {
        retval = temp;
    }

    return retval;
}

QSgmlTag *getXChildrenDown(QSgmlTag *source, uint iDown)
{
    QSgmlTag* retval = nullptr;
    QSgmlTag* temp = source;

    for (auto i = 0u; i < iDown; i++)
    {
        if (temp && temp->Children.size() > 0)
        {
            temp = temp->Children.at(0);
        }
        else
        {
            break;
        }
    }

    if (temp)
    {
        retval = temp;
    }

    return retval;
}

Forum::ForumType getForumType(const QString &strClass)
{
    auto retval = Forum::UNKNOWN;

    if (strClass.indexOf("forum", Qt::CaseInsensitive) != -1)
    {
        retval = Forum::FORUM;
    }
    else if (strClass.indexOf("category", Qt::CaseInsensitive) != -1)
    {
        retval = Forum::CATEGORY;
    }
    else if (strClass.indexOf("link", Qt::CaseInsensitive) != -1 || strClass.indexOf("page", Qt::CaseInsensitive) != -1)
    {
        retval = Forum::LINK;
    }
    else
    {
        const QString error = QString("Xenforo Parser: Unknown node class '%1'").arg(strClass);
        OWL_THROW_EXCEPTION(OwlException(error));
    }

    return retval;
}

QDateTime parseDateTime(QSgmlTag* lastPostDiv, QSgml* doc)
{
    QSgmlTag* timeNode = lastPostDiv->getFirstElementByName("abbr", "class", QRegExp{"DateTime"});
    if (timeNode)
    {
        bool ok = false;
        const QString epochTimeStr = timeNode->getArgValue("data-time");
        const qint64 epochTime = epochTimeStr.toULong(&ok);

        if (ok)
        {
            return QDateTime::fromTime_t(epochTime);
        }
    }
    else
    {
        timeNode = lastPostDiv->getFirstElementByName("span", "class", QRegExp{"DateTime"});
        if (timeNode)
        {
            QString timeStamp = timeNode->getArgValue("title");

            if (timeStamp.isEmpty())
            {
                timeStamp = doc->getText(timeNode);
            }

            if (!timeStamp.isEmpty())
            {
                QDateTime dt;

                std::vector<Qt::DateFormat> formats { Qt::TextDate, Qt::ISODate, Qt::SystemLocaleShortDate,
                    Qt::SystemLocaleLongDate, Qt::DefaultLocaleShortDate, Qt::DefaultLocaleLongDate,
                    Qt::SystemLocaleDate, Qt::LocaleDate, Qt::LocalDate, Qt::RFC2822Date};

                for (const auto dte : formats)
                {
                     dt = QDateTime::fromString(timeStamp, dte);
                     if (dt.isValid())
                     {
                         return dt;
                     }
                }

                // if we're here, there's still no valid date, so let's try a couple formats manually
                const std::vector<QString> vformats = { "MMM d, yyyy 'at' h:mm AP",  "MMM dd, yyyy 'at' hh:mm AP", "MMM dd, yyyy" };
                for (const auto dtf : vformats)
                {
                    dt = QDateTime::fromString(timeStamp, dtf);
                    if (dt.isValid())
                    {
                        return dt;
                    }
                }
            }
        }
    }

    return QDateTime();
}

QString extractMessageText(const QString &rawtext)
{
    QString text = rawtext;

    QSgml doc;
    if (doc.parse(rawtext))
    {
        bool firstRound = true;
        QString quoteAuthor;
        QString quoteText;
        QString postText;

        for (const auto child : doc.DocTag->Children)
        {
            quoteAuthor.clear();
            quoteText.clear();

            if (child->Type == QSgmlTag::eStartTag
                && child->Name == "div"
                && child->getArgValue("class").contains("bbCodeQuote"))
            {
                // if there's a quote, get the author
                QSgmlTag* bbcodequote = child->getFirstElementByName("div", "class", QRegExp{"bbCodeQuote"});
                if (bbcodequote)
                {
                    quoteAuthor = bbcodequote->getArgValue("data-author");
                }

                // if there's a quote, get the quote text
                const auto quotetextNode = child->getFirstElementByName("div", "class", QRegExp{"quote"});
                if (quotetextNode)
                {
                    quoteText = doc.getText(quotetextNode);
                }

                if (!quoteAuthor.isEmpty() && !quoteText.isEmpty())
                {
                    postText += QString("<blockquote><b>%1</b> wrote:<br/><br/>%2</blockquote>")
                        .arg(quoteAuthor)
                        .arg(quoteText);
                }
            }
            else if (child->Type == QSgmlTag::eStartTag
                     && (
                            // there are a few different types of elements we want to ignore
                            (child->Name == "div" && child->getArgValue("class").contains("messageTextEndMarker")) ||
                            (child->Name == "ins" && child->getArgValue("class").toLower() == "adsbygoogle") ||
                            (child->Name == "script")
                         )
                     )
            {
                // skip this
            }
            else if (child->Type == QSgmlTag::eStartTag
                     && child->Name == "strong"
                     && child->getArgValue("class") == "newIndicator")
            {
                // some boards seem to have this in the new posts, so we'll strip it out
            }
            else if (child->Type == QSgmlTag::eStandalone
                     && child->Name == "img")
            {
                // NOTE: we are recreating the <img> tag so that our resizing code will handle it
                const auto srcnode = child->getFirstElementByName("img", "src", QRegExp());
                if (srcnode && !srcnode->getArgValue("src").isEmpty())
                {
                    postText += QString("<img src=\"%1\" onload=\"NcodeImageResizer.createOn(this);\" />")
                        .arg(srcnode->getArgValue("src"));
                }
            }
            else if (child->Type == QSgmlTag::eStartTag
                     && child->Name == "div"
                     && child->getArgValue("class").contains("bbCodeBlock"))
            {
                const auto startPos = firstRound ? child->StartTagPos - 1 : child->StartTagPos;
                const auto endPos = child->EndTagPos - 1;
                const auto divText = rawtext.mid(startPos, endPos - startPos);
                postText += QString("<blockquote>%1</blockquote>")
                    .arg(divText);
            }
            else if (child->Type != QSgmlTag::eVirtualEndTag)
            {
                // HACK: this is a bug in the QSgml parser. If the first element in doc object is a
                // CDATA (text) element, then the start position is f'ed up and the first character
                // will get cut off. So we back the position up one to fix it.
                const auto startPos = firstRound ? child->StartTagPos - 1 : child->StartTagPos;
                const auto nextsibling = child->getNextSibling();

                if (nextsibling)
                {
                    postText += rawtext.mid(startPos, (nextsibling->StartTagPos - startPos));
                }
                else
                {
                    postText += rawtext.mid(startPos, -1);
                }
            }

            firstRound = false;
        }

        if (!postText.isEmpty())
        {
            text = postText;
        }
    }

    return text;
}

} // namespace
