// Owl - www.owlclient.com
// Copyright (c) 2012-2019, Adalid Claure <aclaure@gmail.com>

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

std::uint32_t Xenforo2::parsingScore(const owl::LoginInfo& login)
{
    std::uint32_t score = 0;

    return score;
}

} // namespace owl