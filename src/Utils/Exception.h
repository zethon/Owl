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
    {
    }

	OwlException(const OwlException& other)
        : boost::exception(other),
          QException(other)
	{
        _message = other._message;
	}

	virtual QString message() const { return _message; }
    
    virtual QString details() const
    {
        QString retval;

        const QString fn { filename() };
        const auto ln { line() };
        
        if (fn.length() > 0 && ln > 0)
        {
            retval = QString("[%1:%2] %3")
                .arg(fn)
                .arg(ln)
                .arg(_message);
        }
        else
        {
            retval.append(_message);
        }
        
        return retval;
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
	QString			_message;
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

    virtual ~WebException() throw() = default;

    WebException(const QString& msg, const QString& lastUrl = QString(), int statusCode = -1)
        : OwlException(msg),
          _lastUrl(lastUrl), 
          _statusCode(statusCode)
    {}
      
    virtual QString details() const override
    {
        QString retval = OwlException::details();
        
        retval.append(QString(" (%1) [Status Code: %2]").arg(_lastUrl).arg(_statusCode));
        
        return retval;
    }

	const QString& url() const throw() { return _lastUrl; }
	const int statuscode() const throw() { return _statusCode; }
    
private:
    QString _lastUrl;
    int     _statusCode;    
};

class LuaException : public OwlException
{

public:
    LuaException(const QString& msg);
    virtual ~LuaException() throw() {}

    const QString& luaError() const throw();

private:
	QString _luaError;
};

} // namespace

Q_DECLARE_METATYPE(owl::OwlException)
