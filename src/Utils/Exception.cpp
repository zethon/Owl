// Owl - www.owlclient.com
// Copyright (c) 2012-2018, Adalid Claure <aclaure@gmail.com>

#include "Exception.h"

namespace owl
{

ConfigureException::ConfigureException(const QString &msg)
    : OwlException(msg)
{
    // do nothing
}

FormatException::FormatException(const QString &msg, const QString &filename, int line)
    : OwlException(msg, filename, line)
{
    // do nothing
}

LuaException::LuaException(const QString &msg)
    : OwlException(msg)
{
    // nothing to do
}

const QString &LuaException::luaError() const throw()
{
    return _luaError;
}

} // namespace
