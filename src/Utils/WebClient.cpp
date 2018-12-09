// Owl - www.owlclient.com
// Copyright (c) 2012-2018, Adalid Claure <aclaure@gmail.com>

#include <tidy.h>
#include <tidybuffio.h>
#include "WebClient.h"

#include <Utils/OwlLogger.h>

namespace owl
{

static size_t CURLwriter(char *data, size_t size, size_t nmemb, std::string *writerData)
{
    if (writerData == nullptr)
    {
        return 0;
    }

    writerData->append(data, size*nmemb);
    return size * nmemb;
}

//int trace(CURL *handle, curl_infotype type, unsigned char *data, size_t size, void *userp)
//{
//    std::cout << data << std::endl;
//    return 1;
//}

CURLcode curlGlobalInit()
{
    return curl_global_init(CURL_GLOBAL_ALL);
}

WebClient::WebClient()
    : _logger(owl::initializeLogger("Xenforo"))
{
    static CURLcode __global = curlGlobalInit();
    Q_UNUSED(__global)

    _curl = curl_easy_init();

    if (_curl)
    {
        _textCodec = QTextCodec::codecForName("Windows-1251");

        // setup the _cur object
        initCurlSettings();
    }
    else
    {
        OWL_THROW_EXCEPTION(OwlException("Could not create CURL instance"));
    }
}

WebClient::~WebClient()
{
    curl_easy_cleanup(_curl);
}

void WebClient::setUserAgent(const QString& agent)
{
    Lock lock(_curlMutex);
    curl_easy_setopt(_curl, CURLOPT_USERAGENT, agent.toLocal8Bit().data());
}

void WebClient::setHeader(const QString &key, const QString val)
{
    _headers.setOrAdd(key, val);
}

void WebClient::clearHeaders()
{
    _headers.clear();
}

void WebClient::addSendCookie(const QString &key, const QString &value)
{
    Lock lock(_curlMutex);

    const QString data = QString("%1=%2").arg(key).arg(value);
    curl_easy_setopt(_curl, CURLOPT_COOKIE, data.toLatin1().data());
}

void WebClient::eraseSendCookies()
{
    Lock lock(_curlMutex);
    curl_easy_setopt(_curl, CURLOPT_COOKIE, "");
}

void WebClient::printCookies()
{
    CURLcode res;
    struct curl_slist *cookies;
    struct curl_slist *nc;
    int i;

    printf("Cookies, curl knows:\n");
    res = curl_easy_getinfo(_curl, CURLINFO_COOKIELIST, &cookies);
    if(res != CURLE_OK)
    {
      fprintf(stderr, "Curl curl_easy_getinfo failed: %s\n",
              curl_easy_strerror(res));
      exit(1);
    }

    nc = cookies, i = 1;

    while(nc)
    {
      printf("[%d]: %s\n", i, nc->data);
      nc = nc->next;
      i++;
    }

    if(i == 1)
    {
      printf("(none)\n");
    }
    curl_slist_free_all(cookies);
}

void WebClient::deleteAllCookies()
{
    Lock lock(_curlMutex);
    curl_easy_setopt(_curl, CURLOPT_COOKIELIST, "ALL");
}

void WebClient::setCurlHandle(CURL *curl)
{
    Lock lock(_curlMutex);

    // release the existing instance
    curl_easy_cleanup(_curl);

    // make a duplicate of the handle
    _curl = curl_easy_duphandle(curl);

    // seems that duplication only duplicates some things so initialize this object too
    initCurlSettings();

    // copy the cookies
    struct curl_slist *cookies;
    curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
    struct curl_slist* nc = cookies;

    while (nc)
    {
        curl_easy_setopt(_curl, CURLOPT_COOKIELIST, nc->data);
        nc = nc->next;
    }

    curl_slist_free_all(cookies);
}

const QString WebClient::getLastRequestUrl() const
{
    return _lastUrl;
}

void WebClient::setConfig(const WebClientConfig &config)
{
    Lock lock(_curlMutex);

    _useEncryption = config.useEncryption;
    _strEncryptionSeed = config.encryptSeed;
    _strEncyrptionKey = config.encryptKey;

    // use the actual call instead of setUserAgent() to avoid deadlock and having to
    // use a recursive mutex
    curl_easy_setopt(_curl, CURLOPT_USERAGENT, config.userAgent.toLocal8Bit().data());
}

QString WebClient::DownloadString(const QString &url, uint options /*=Options::DEFAULT*/)
{
    const auto reply = GetUrl(url, options);

    if (reply)
    {
        return reply->text();
    }

    return QString();
}

WebClient::ReplyPtr WebClient::GetUrl(const QString &url, uint options)
{
    return doRequest(url, QString(), Method::GET, options);
}

QString WebClient::UploadString(const QString& url, const QString &payload, uint options)
{
    const auto reply = PostUrl(url, payload, options);

    if (reply)
    {
        return reply->text();
    }

    return QString();
}

WebClient::ReplyPtr WebClient::PostUrl(const QString& url, const QString &payload, uint options)
{
    return doRequest(url, payload, Method::POST, options);
}

WebClient::ReplyPtr WebClient::doRequest(const QString& url,
                                   const QString& payload /*= QString()*/,
                                   Method method /*= Method::GET*/,
                                   uint options /*= Options::DEFAULT*/)
{
    Lock lock(_curlMutex);
    QTime timer;
    timer.start();

    bool bThrowOnFail = getThrowOnFail();

    // set the URL we're getting
    curl_easy_setopt(_curl, CURLOPT_URL, url.toLatin1().data());

    // set up a GET or POST, if not a GET assume a POST
    if (method == Method::GET)
    {
        QUrl urlObj = QUrl::fromUserInput(url);

        _logger->debug("Running {} GET request of url '{}'",
            urlObj.scheme().toUpper().toStdString(),
            url.toStdString());

        curl_easy_setopt(_curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(_curl, CURLOPT_POST, 0L);
    }
    else if (method == Method::POST)
    {
        QUrl urlObj = QUrl::fromUserInput(url);

        _logger->debug("Running {} POST request for url '{}' with payload size of {} bytes",
            urlObj.scheme().toUpper().toStdString(),
            url.toStdString(),
            payload.size());

        curl_easy_setopt(_curl, CURLOPT_HTTPGET, 0L);
        curl_easy_setopt(_curl, CURLOPT_POST, 1L);

        if (payload.size() > 0)
        {
            curl_easy_setopt(_curl, CURLOPT_POSTFIELDSIZE, payload.size());
            curl_easy_setopt(_curl, CURLOPT_COPYPOSTFIELDS, payload.toLocal8Bit().data());
        }
        else
        {
            curl_easy_setopt(_curl, CURLOPT_POSTFIELDSIZE, 0L);
            curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, nullptr);
        }
    }
    else
    {
        OWL_THROW_EXCEPTION(owl::WebException("Unsupported HTTP method"));
    }

    auto headers = setHeaders();

    _buffer.clear();
    CURLcode result = curl_easy_perform(_curl);

    unsetHeaders(headers);

    long status = 0;
    curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &status);

    if (result != CURLE_OK)
    {
        QString errorText;

        size_t len = strlen(_errbuf);
        if (len > 0)
        {
            errorText = QString("Request error: %1")
                .arg(_errbuf);
        }
        else
        {
            errorText = QString("Request error: %1")
                .arg(curl_easy_strerror(result));
        }

        _logger->warn(errorText.toStdString());
        if (bThrowOnFail)
        {
            // TODO: Add more details to this exception, like below:
            OWL_THROW_EXCEPTION(owl::WebException(errorText, url, status));
        }

        return nullptr;
    }

    char *finalUrl;
    curl_easy_getinfo(_curl, CURLINFO_EFFECTIVE_URL, &finalUrl);

    _lastUrl = QString::fromLatin1(finalUrl);

    auto retval = std::make_shared<Reply>(status);
    retval->setFinalUrl(finalUrl);

    if (status == 200)
    {
        if (!(options & Options::NOTIDY))
        {
            std::string temp{ owl::tidyHTML(_buffer.c_str()) };
            retval->setData(temp, temp.size());
        }
        else
        {
            retval->setData(_buffer, _buffer.size());
        }

        _logger->trace("HTTP Response from '{}' with length of '{}' took {} milliseconds",
            finalUrl, _buffer.size(), timer.elapsed());
    }
    else
    {
        QString errorText = QString("Unhandled HTTP response code '%1' from %2 took %3 milliseconds").arg(status).arg(finalUrl).arg(timer.elapsed());
        _logger->debug(errorText.toStdString());
        if (bThrowOnFail)
        {
            OWL_THROW_EXCEPTION(owl::WebException(errorText, url, status));
        }
        else
        {
            // sometimes the data is still needed even if we don't get
            // a 200 result, but we can safely NOT tidy it
            retval->setData(_buffer, _buffer.size());
        }
    }

    return retval;
}

curl_slist* WebClient::setHeaders()
{
	curl_slist* headers = nullptr;

    // add out content type
    const QString contentType = QString("Content-Type: %1").arg(_contentType);
    headers = curl_slist_append(headers, contentType.toLatin1().data());

    for (const auto& kv : _headers)
    {
        const QString header = QString("%1: %2")
            .arg(kv.first)
            .arg(kv.second);

        headers = curl_slist_append(headers, header.toLatin1().data());
    }

    /* pass our list of custom made headers */
    curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, headers);

    return headers;
}

void WebClient::unsetHeaders(curl_slist* headers)
{
    /* free the header list */
    curl_slist_free_all(headers);
}

void WebClient::initCurlSettings()
{
    curl_version_info_data *vinfo = curl_version_info(CURLVERSION_NOW);
    if (!(vinfo->features & CURL_VERSION_SSL))
    {
        OWL_THROW_EXCEPTION(OwlException("CURL SSL support is required but not enabled"));
    }

    // set up our writer
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, CURLwriter);
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &_buffer);
    curl_easy_setopt(_curl, CURLOPT_ERRORBUFFER, _errbuf);

    // set the redirects and the max number
    curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(_curl, CURLOPT_MAXREDIRS, DEFAULT_MAX_REDIRECTS);

    // start cookie engine
    curl_easy_setopt(_curl, CURLOPT_COOKIEFILE, "");

    // <SSL CONFIG>
    // since PEM is default, we needn't set it for PEM
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    // need to disable this otherwise SSL does not work on Windows 7
    curl_easy_setopt(_curl, CURLOPT_SSL_ENABLE_ALPN, 0);

    // tell libcurl to redirect a post with a post after a 301, 302 or 303
    curl_easy_setopt(_curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);

    // disable all curl's signal handling
    curl_easy_setopt(_curl, CURLOPT_NOSIGNAL, 1L);

//#ifdef _DEBUG
//    curl_easy_setopt(_curl, CURLOPT_VERBOSE, 1);
//    curl_easy_setopt(_curl, CURLOPT_DEBUGFUNCTION, trace);
//#endif
}

const std::string tidyHTML(const std::string& html)
{
    // see:http://tidy.sourceforge.net/libintro.html
    TidyDoc tdoc = tidyCreate();
    TidyBuffer output = {0};
    TidyBuffer errbuf = {0};
    Bool ok;
    int rc = -1;
    std::string retStr;

    tidyOptSetBool(tdoc, TidyMark, no);
    tidyOptSetInt(tdoc, TidyWrapLen, 0);

    ok = tidyOptSetBool( tdoc, TidyXhtmlOut, yes );  // Convert to XHTML
    if (ok)
        rc = tidySetErrorBuffer( tdoc, &errbuf );      // Capture diagnostics (required!)

    if ( rc >= 0 )
        rc = tidyParseString( tdoc, html.data() );           // Parse the input

    if ( rc >= 0 )
        rc = tidyCleanAndRepair( tdoc );               // Tidy it up!

    //if ( rc >= 0 )
    //    rc = tidyRunDiagnostics( tdoc );               // Kvetch

    //if ( rc > 1 )                                    // If error, force output.
    //    rc = ( tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1 );

    if ( rc >= 0 )
        rc = tidySaveBuffer( tdoc, &output );          // Pretty Print

    if ( rc >= 0 )
    {
        std::string str(reinterpret_cast<char const*>(output.bp), output.size);
        retStr = std::move(str);
    }
    else
    {
        // TODO: this error probably shouldn't go unnoticed

        // bail out and return the original string
        retStr = html;
    }

    tidyBufFree(&output);
    tidyBufFree(&errbuf);
    tidyRelease(tdoc);

    return retStr;
}

} // namespace
