#pragma once

#include <QString>

namespace owl::literals
{

QString operator""_qs(const char* str, std::size_t)
{
    return QString::fromLatin1(str);
}

} // namespace owl
