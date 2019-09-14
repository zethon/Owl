#pragma once
#include <QtCore>

#include "../Utils/QSgml.h"

#include "ParserBase.h"

class QSgml;
class QSgmlTag;

namespace spdlog
{
    class logger;
}

namespace owl
{

#define XENFORO2_NAME		"xenforo2x"
#define XENFORO2_PRETTYNAME  "XenForo 2.x"

class WebClient;

class Xenforo2;
using Xenforo2Ptr = std::shared_ptr<Xenforo2>;

class Xenforo2 : public ParserBase
{
	Q_OBJECT

public:

    Q_INVOKABLE Xenforo2(const QString& url);
    virtual ~Xenforo2() = default;

    QString getLastRequestUrl() override
    {
        return QString{};
    }

    std::uint32_t parsingScore(const owl::LoginInfo& url);

protected:

    QVariant doLogin(const LoginInfo&) override;

    QVariant doLogout() override
    {
        return QVariant{};
    }

    virtual QVariant doGetBoardwareInfo() override
    {
        return QVariant{};
    }

    QVariant doTestParser(const QString&) override
    {
        return QVariant{};
    }

    QVariant doGetForumList(const QString& forumId) override
    {
        return QVariant{};
    }

    QVariant doThreadList(ForumPtr forumInfo, int options) override
    {
        return QVariant{};
    }

    QVariant doGetPostList(ThreadPtr t, PostListOptions listOption, int webOptions) override
    {
        return QVariant{};
    }

    QVariant doSubmitNewThread(ThreadPtr threadInfo) override
    {
        return QVariant{};
    }

    QVariant doSubmitNewPost(PostPtr postInfo) override
    {
        return QVariant{};
    }

    QVariant doGetEncryptionSettings() override
    {
        return QVariant{};
    }

private:
    owl::WebClient                      _webclient;
    std::shared_ptr<spdlog::logger>     _logger;
    QSgml                               _parser;

};

} // namespace owl