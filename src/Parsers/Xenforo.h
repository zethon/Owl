#pragma once
#include <QtCore>
#include <log4qt/logger.h>
#include "ParserBase.h"

class QSgml;
class QSgmlTag;

namespace owl
{

#define XENFORO_NAME		"xenforo1x"
#define XENFORO_PRETTYNAME  "XenForo 1.x"

class WebClient;

class Xenforo;
using XenforoPtr = std::shared_ptr<Xenforo>;

class Xenforo : public ParserBase
{
	Q_OBJECT
	LOG4QT_DECLARE_QCLASS_LOGGER

public:

    Q_INVOKABLE Xenforo(const QString& url);
    virtual ~Xenforo() = default;

    virtual ParserBasePtr clone(ParserBasePtr other = ParserBasePtr()) override;

    virtual bool canParse(const QString &) override;

    virtual QString getPostQuote(PostPtr post) override;

    virtual QString getLastRequestUrl() override
    {
        return _webclient.getLastRequestUrl();
    }

    // See ParserBase.h for more explanation about these two methods
    virtual const std::pair<uint, bool> defaultPostsPerPage() const override { return std::make_pair(20, true); }
    virtual const std::pair<uint, bool> defaultThreadsPerPage() const override { return std::make_pair(20, true); }

protected:
    virtual QVariant doLogin(const LoginInfo&) override;
    virtual QVariant doLogout() override;

    virtual QVariant doGetBoardwareInfo() override;
    virtual QVariant doTestParser(const QString&) override { return QVariant(); }

    virtual QVariant doGetForumList(const QString& forumId) override;
    virtual QVariant doThreadList(ForumPtr forumInfo, int options) override;
    virtual QVariant doGetPostList(ThreadPtr t, PostListOptions listOption, int webOptions) override;

    virtual QVariant doSubmitNewThread(ThreadPtr threadInfo) override;
    virtual QVariant doSubmitNewPost(PostPtr postInfo) override;

    virtual QVariant doGetUnreadForums() override;
    virtual QVariant doMarkForumRead(ForumPtr forumInfo) override;

    virtual QVariant doGetEncryptionSettings() override { return QVariant(); }

private:
    ForumList   getRootForumPrivate();
    ForumList   getForumsPrivate(const QString& id);

    WebClient               _webclient;
    QString                 _logoutUrl;
};

QSgmlTag*           getXParentsUp(QSgmlTag* source, uint i);
QSgmlTag*           getXChildrenDown(QSgmlTag* source, uint i);
Forum::ForumType    getForumType(const QString& strType);
QDateTime           parseDateTime(QSgmlTag*, QSgml* doc);
QString             extractMessageText(const QString& rawtext);

} // namespace
