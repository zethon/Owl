// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

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
#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/errinfo_at_line.hpp>

namespace owl
{

template<class E>
void ThrowException(const E & ex, const char * function, int line)
{
	throw (ex << boost::throw_function(function) << boost::errinfo_at_line(line));
}

#define OWL_THROW_EXCEPTION(x)      ThrowException((x), BOOST_CURRENT_FUNCTION, __LINE__);

class OwlException :
        public boost::exception,
        public QException
{
public:
	OwlException() 
	{ 
    }

    OwlException(const QString msg)
        : _message(msg), _line(0)
    {
    }

    OwlException(const QString msg, const QString& filename)
        : _message(msg), _filename(filename), _line(0)
    {
    }

    OwlException(const QString msg, const QString& filename, int line)
		: _message(msg), _filename(filename), _line(line)
	{
	}

	OwlException(const OwlException& other)
	{
		_message = other.message();
		_filename = other.filename();
		_line = other.line();
	}

	OwlException& operator=(const OwlException& other)
	{
		_message = other.message();
		_filename = other.filename();
		_line = other.line();
        return *this;
	}

	virtual ~OwlException() throw()
	{
	}

	virtual void raise() const { throw *this; }
	virtual OwlException* clone() const { return new OwlException(*this); }
	
	virtual QString filename() const { return _filename; }
	virtual int line()  const { return _line; }

	virtual QString message() const { return _message; }
    
    virtual QString details() const
    {
        QString retval;
        
        if (_filename.length() > 0 && _line > 0)
        {
            retval = QString("[%1:%2] %3")
                .arg(_filename)
                .arg(_line)
                .arg(_message);
        }
        else
        {
            retval.append(_message);
        }
        
        return retval;
    }
    
	virtual operator std::string() const { return this->message().toStdString(); }
	virtual operator QString() const { return this->message(); }

private:
	QString			_message;
	QString			_filename;
	int				_line;
};

class NotImplementedException final : public OwlException
{

public:
    NotImplementedException(const QString msg)
        : OwlException(msg)
	{
        // do nothing
    }

    virtual ~NotImplementedException() = default;
};

class WebException : public OwlException
{
    
public:
    WebException(const QString& msg, const QString& lastUrl = QString(), int statusCode = -1)
        : WebException(msg, lastUrl, statusCode, QString(), -1)
    {
    }
    
	WebException(const QString& msg, const QString& lastUrl, const QString& filename, int line)
		: OwlException(msg, filename, line),
		  _lastUrl(lastUrl),
		  _statusCode(-1)
	{
	}
    
	WebException(const QString& msg, const QString& lastUrl, int statusCode, const QString& filename, int line)
		: OwlException(msg, filename, line),
		  _lastUrl(lastUrl),
		  _statusCode(statusCode)
	{
	}
    
    virtual ~WebException() throw()
	{
	}
    
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

class ConfigureException : public OwlException
{

public:
    ConfigureException(const QString& msg);
    virtual ~ConfigureException() throw() {}
};

class FormatException : public OwlException
{

public:
    FormatException(const QString& msg, const QString& filename, int line);
    virtual ~FormatException() throw() {}
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

typedef std::shared_ptr<owl::OwlException>	OwlExceptionPtr;

} // namespace

Q_DECLARE_METATYPE(owl::OwlException)
Q_DECLARE_METATYPE(owl::OwlExceptionPtr)
