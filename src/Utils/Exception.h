#pragma once

#ifdef _WINDOWS
#pragma warning(disable:4530)
#endif

#include <stdexcept>
#include <string>
#include <sstream>
#include <QtCore>
#include <QException>

#include <boost/exception/all.hpp>
#include <boost/stacktrace.hpp>


#include <iostream>

namespace owl
{

using traced = boost::error_info<struct tag_stacktrace, boost::stacktrace::stacktrace>;

template<class E>
void ThrowException(const E & ex, const char * function, const char* filename, int line)
{
    throw boost::enable_error_info(ex)
            << boost::throw_function(function)
            << boost::throw_file(filename)
            << boost::throw_line(line)
            << traced(boost::stacktrace::stacktrace());
}

#define OWL_THROW_EXCEPTION(x)  ThrowException((x), BOOST_CURRENT_FUNCTION, __FILE__, __LINE__);

class OwlException :
        public boost::exception,
        public QException
{
public:

    virtual ~OwlException() noexcept = default;

	OwlException() = default;

    OwlException(const QString& msg)
        : _message(msg)
    {}

	OwlException(const OwlException& other)
        : boost::exception(other),
          QException(other),
          _message(other._message)
    {}

    virtual QString message() const noexcept { return _message; }

    virtual QString details() const
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

    [[nodiscard]] std::int32_t line() const
    {
        if (auto ln = boost::get_error_info<boost::throw_line>(*this); ln)
        {
            return *ln;
        }

        return -1;
    }

    [[nodiscard]] QString filename() const
    {
        if (auto fn = boost::get_error_info<boost::throw_file>(*this); fn)
        {
            return QString::fromStdString(*fn);
        }

        return QString{};
    }


    [[nodiscard]] QString function() const
    {
        if (auto fn = boost::get_error_info<boost::throw_function>(*this); fn)
        {
            return *fn;
        }

        return QString{};
    }

private:
    QString     _message;
};

class NotImplementedException final : public OwlException
{

public:
    virtual ~NotImplementedException() = default;

    using OwlException::OwlException;
};

class WebException : public OwlException
{
    
public:

    virtual ~WebException() = default;

    WebException(const QString& msg, const QString& lastUrl = QString(), int statusCode = -1)
        : OwlException(msg),
          _lastUrl(lastUrl), 
          _statusCode(statusCode)
    {}

    QString lastUrl() const { return _lastUrl; }
    std::int32_t statuscode() const { return _statusCode; }

    QString details() const override
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

        return QString::fromStdString(ss.str()) + OwlException::details();
    }
    
private:
    QString         _lastUrl;
    std::int32_t    _statusCode;
};

class LuaException : public OwlException
{

public:
    virtual ~LuaException() = default;

    LuaException(const QString& msg)
        : OwlException(msg)
    {
        // nothing to do
    }

    QString luaError() const { return _luaError; }

private:
	QString _luaError;
};

} // namespace

Q_DECLARE_METATYPE(owl::OwlException)
