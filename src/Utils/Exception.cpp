// Owl - www.owlclient.com
// Copyright (c) 2012-2023, Adalid Claure <aclaure@gmail.com>

#include "Exception.h"

namespace owl
{

QString Exception::details() const
{
    std::stringstream ss;

    if (auto fn = boost::get_error_info<boost::throw_function>(*this); fn)
    {
        ss << "Function: " << *fn << '\n';
    }
    else
    {
        ss << "Function: unknown";
    }

    if (auto fn = boost::get_error_info<boost::throw_file>(*this); fn)
    {
        const QFileInfo temp { QString::fromStdString(*fn) };
        ss << "Source file: " << temp.fileName().toStdString();

        if (auto ln = boost::get_error_info<boost::throw_line>(*this); ln)
        {
            ss << ":" << *ln;
        }

        ss << '\n';
    }
    else
    {
        ss << "File: unknown\n";
    }

    if (auto st = boost::get_error_info<traced>(*this); st)
    {
        ss << '\n' << *st << '\n';
    }

    return QString::fromStdString(ss.str());
}

std::int32_t Exception::line() const
{
    if (auto ln = boost::get_error_info<boost::throw_line>(*this); ln)
    {
        return *ln;
    }

    return -1;
}

QString Exception::filename() const
{
    if (auto fn = boost::get_error_info<boost::throw_file>(*this); fn)
    {
        return QString::fromStdString(*fn);
    }

    return QString{};
}

QString Exception::function() const
{
    if (auto fn = boost::get_error_info<boost::throw_function>(*this); fn)
    {
        return *fn;
    }

    return QString{};
}

WebException::WebException(const QString& msg, const QString& lastUrl, int statusCode)
    : Exception(msg),
      _lastUrl(lastUrl),
      _statusCode(statusCode)
{}

QString WebException::details() const
{
    std::stringstream ss;

    if (_lastUrl.size() > 0)
    {
        ss << "Last URL: " << _lastUrl.toStdString() << '\n';
    }

    if (_statusCode > 0)
    {
        ss << "Status Code: " << _statusCode << '\n';
    }

    return QString::fromStdString(ss.str()) + Exception::details();
}


LuaException::LuaException(const QString& msg)
    : Exception(msg)
{
    // nothing to do
}

} // namespace
