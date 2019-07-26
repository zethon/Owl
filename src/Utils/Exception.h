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

class Exception :
        public boost::exception,
        public QException
{
public:

    virtual ~Exception() noexcept = default;

    Exception() = default;

    Exception(const QString& msg)
        : _message(msg)
    {}

    Exception(const Exception& other)
        : boost::exception(other),
          QException(other),
          _message(other._message)
    {}

    virtual QString message() const noexcept { return _message; }

    virtual QString details() const;

    [[nodiscard]] std::int32_t line() const;

    [[nodiscard]] QString filename() const;

    [[nodiscard]] QString function() const;

private:
    QString     _message;
};

class NotImplementedException final : public Exception
{

public:
    virtual ~NotImplementedException() = default;

    using Exception::Exception;
};

class WebException : public Exception
{
    
public:

    WebException(const QString& msg, const QString& lastUrl = QString(), int statusCode = -1);

    QString lastUrl() const { return _lastUrl; }
    std::int32_t statuscode() const { return _statusCode; }

    QString details() const override;
    
private:
    QString         _lastUrl;
    std::int32_t    _statusCode;
};

class LuaException : public Exception
{

public:
    virtual ~LuaException() = default;

    LuaException(const QString& msg);

    QString luaError() const { return _luaError; }

private:
	QString _luaError;
};

} // namespace

Q_DECLARE_METATYPE(owl::Exception)
