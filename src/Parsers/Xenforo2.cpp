// Owl - www.owlclient.com
// Copyright (c) 2012-2019, Adalid Claure <aclaure@gmail.com>

#include <QClipboard>
#include <QApplication>

#include "../Utils/OwlUtils.h"
#include "../Utils/QSgml.h"
#include "../Utils/QSgmlTag.h"
#include "../Utils/WebClient.h"
#include "Xenforo2.h"
#include <cmath>

#include <Utils/OwlLogger.h>

namespace owl
{

Q_INVOKABLE Xenforo2::Xenforo2(const QString &baseUrl)
    : ParserBase(XENFORO2_NAME,XENFORO2_PRETTYNAME, baseUrl),
      _logger(owl::initializeLogger("Xenforo2"))
{
    addWatcher(&_webclient);
}

std::uint32_t Xenforo2::parsingScore(const owl::LoginInfo& creds)
{
    std::uint32_t score = 0;

    if (!creds.empty())
    {
        auto resvar = this->doLogin(creds);
        assert(resvar.canConvert<owl::StringMap>());

        auto results = resvar.value<owl::StringMap>();
        if (results.has("score"))
        {
            score += results.get<std::uint32_t>("score", false);
        }
    }

    return score;
}

QVariant Xenforo2::doLogin(const LoginInfo& creds)
{
    _logger->trace("Logging into {} with username {}", getBaseUrl().toStdString(), creds.login().toStdString());
    const QString loginUrl{ this->getBaseUrl() + "/login/login" };

    owl::StringMap params;
    params.add("login", creds.login());
    params.add("password", creds.password());

    // and other params we want to pass
    params.add("cookie_check", "0");
    params.add("remember", "1");
    params.add("_xfToken", QString());
    
    QString data = _webclient.UploadString(loginUrl, params.encode());

    if (_parser.parse(data))
    {
        QApplication::clipboard()->setText(data);
    }
    
    owl::StringMap retval;
    return QVariant::fromValue(retval);
}

} // namespace owl