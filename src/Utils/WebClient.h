// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <mutex>
#include <logger.h>
#include "StringMap.h"

#include <curl.h>

namespace owl
{

const QString   DEFAULT_CONTENT_TYPE	= "application/x-www-form-urlencoded";
const uint      DEFAULT_MAX_REDIRECTS	= 5;

struct WebClientConfig
{
	QString userAgent;
    
    bool    useEncryption;
    QString encryptKey;
    QString encryptSeed;
};

struct HttpReply
{
    QString     data;           // the body of the reply
    QString     finalUrl;       // the final url that sent the response (redirects & rewrites)
    long        status = -1;    // the http status
};
using HttpReplyPtr = std::shared_ptr<HttpReply>;

class WebClient :  public QObject
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER

    using Mutex         = std::mutex;
    using Lock          = std::lock_guard<std::mutex>;
    using UniqueLock    = std::unique_lock<std::mutex>;

public:

    enum Method
    {
        GET     = 1,
        POST    = 2
    };

    enum Options
    {
        DEFAULT		= 0x0000,
        NOTIDY		= 0x0001,
        NOCACHE		= 0x0002,
        NOENCRYPT   = 0x0004
    };

    WebClient();
    virtual ~WebClient();

    bool getThrowOnFail() const { return _throwOnFail; }
    void setThrowOnFail(bool var) { _throwOnFail = var; }

    QString getContentType() const { return _contentType; }
    void setContentType(const QString& var) { _contentType = var; }

    void setUserAgent(const QString&);

    void setHeader(const QString& key, const QString val);
    void clearHeaders();

    // set the CURL instance
    void setCurlHandle(CURL* curl);
    CURL* getCurlHandle() const { return _curl; }

    // The effective URL of the last successful request
    const QString getLastRequestUrl() const;

    void setConfig(const WebClientConfig& config);

    void addSendCookie(const QString& key, const QString& value);
    void eraseSendCookies();
    void printCookies();
    void deleteAllCookies();

    // Submits an HTTP GET and returns the webpage contents or an empty string
    QString DownloadString(const QString& url, uint options = Options::DEFAULT, const StringMap& params = StringMap());

    // Submits an HTTP GET and returns a reply object or NULL
    HttpReplyPtr GetUrl(const QString& url, uint options = Options::DEFAULT, const StringMap& params = StringMap());

    // Submits an HTTP POST and returns the result's string or an empty string
    QString UploadString(const QString& address, const QString& payload, uint options = Options::DEFAULT, const StringMap& params = StringMap());

    // Submits an HTTP POST and returns a reply object or NULL
    HttpReplyPtr PostUrl(const QString& url, const QString& payload, uint options = Options::DEFAULT, const StringMap& params = StringMap());

    // If successful, will return a new object and release ownership to the caller
    // If unsucessful, throw an error OR return null if throwOnFail=false
    HttpReplyPtr doRequest(const QString& url,
                           const QString& payload = QString(),
                           Method method = Method::GET,
                           uint options = Options::DEFAULT,
                           const StringMap& params = StringMap());
private:
    curl_slist* setHeaders();
	void unsetHeaders(curl_slist* headers);
    void initCurlSettings();

    Mutex               _curlMutex;

    CURL*               _curl = nullptr;                        // the curl object
    std::string         _buffer;                                // buffer for response text
    char                _errbuf[CURL_ERROR_SIZE];               // detailed error buffer
    StringMap           _headers;                               // map of headers that get set before requests and unset after
    QTextCodec*         _textCodec;                             // TODO: learn more about this
    QString             _lastUrl;                               // realized url from the last successful request
    QString             _contentType = "application/x-www-form-urlencoded";

    bool                _throwOnFail = true;                   // whether or not to throw, can be overriden in actual call

    bool                _useEncryption = false;                 // whether or not to encrypt the result, can be overridden in actual call
    QString             _strEncyrptionKey;
    QString             _strEncryptionSeed;
};

const QString tidyHTML(const QString& html);

} // namespace
