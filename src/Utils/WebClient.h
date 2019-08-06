// Owl - www.owlclient.com
// Copyright (c) 2012-2019, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <mutex>
#include "StringMap.h"

#include <curl/curl.h>

namespace spdlog
{
    class logger;
}

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

class WebClient :  public QObject
{
    Q_OBJECT

    using Mutex         = std::mutex;
    using Lock          = std::lock_guard<std::mutex>;
    using UniqueLock    = std::unique_lock<std::mutex>;

public:
    class Reply
    {
        long            _status = -1;    // the http status
        std::string     _data;
        std::string     _finalUrl;       // the final url that sent the response (redirects & rewrites)

        public:
            Reply(long status)
                : _status { status }
            {}

            long status() const { return _status; }
            void setStatus(long status) { _status = status; }

            QString text() const { return QString::fromStdString(_data); }

            std::string const data() { return _data; }
            void setData(const std::string& data, std::size_t size) 
            { 
                _data.append(data.data(), size); 
            }

            std::string finalUrl() const { return _finalUrl; }
            void setFinalUrl(const std::string& finalUrl) { _finalUrl = finalUrl; }
    };
    using ReplyPtr = std::shared_ptr<Reply>;

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
    QString DownloadString(const QString& url, uint options = Options::DEFAULT);

    // Submits an HTTP GET and returns a reply object or nullptr
    ReplyPtr GetUrl(const QString& url, uint options = Options::DEFAULT);

    // Submits an HTTP POST and returns the result's string or an empty string
    QString UploadString(const QString& address, const QString& payload, uint options = Options::DEFAULT);

    // Submits an HTTP POST and returns a reply object or nullptr
    ReplyPtr PostUrl(const QString& url, const QString& payload, uint options = Options::DEFAULT);

private:
    // If successful, will return a new object and release ownership to the caller
    // If unsucessful, throw an error OR return null if throwOnFail=false
    ReplyPtr doRequest(const QString& url,
                           const QString& payload = QString(),
                           Method method = Method::GET,
                           uint options = Options::DEFAULT);

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

    std::shared_ptr<spdlog::logger>  _logger;
};

const std::string tidyHTML(const std::string& html);

} // namespace
