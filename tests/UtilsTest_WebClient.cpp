// Owl - www.owlclient.com
// Copyright (c) 2012-2019, Adalid Claure <aclaure@gmail.com>
#include <QCryptographicHash>

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <iostream>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "../src/Utils/WebClient.h"

namespace data = boost::unit_test::data;

BOOST_AUTO_TEST_SUITE(WebClientTests)

std::tuple<const char*, const char*, long> statusData[]
{
    std::tuple<const char*, const char*, long>
    {
        "https://httpstat.us/200",
        "200 OK",
        200
    },
    std::tuple<const char*, const char*, long>
    {
        "https://httpstat.us/201",
        "201 Created",
        201
    },
    std::tuple<const char*, const char*, long>
    {
        "https://httpstat.us/202",
        "202 Accepted",
        202
    },
    std::tuple<const char*, const char*, long>
    {
        "https://httpstat.us/404",
        "404 Not Found",
        404
    },
    std::tuple<const char*, const char*, long>
    {
        "https://httpstat.us/429",
        "429 Too Many Requests",
        429
    },
    std::tuple<const char*, const char*, long>
    {
        "https://httpstat.us/502",
        "502 Bad Gateway",
        502
    }
};

BOOST_DATA_TEST_CASE(statusTests, data::make(statusData), url, expectedResponse, expectedStatus)
{
    owl::WebClient client;
    client.setThrowOnFail(false);
    auto reply = client.GetUrl(QString::fromLatin1(url), owl::WebClient::NOTIDY | owl::WebClient::NOCACHE);
    BOOST_REQUIRE(reply != nullptr);
    BOOST_CHECK_EQUAL(reply->text().toStdString(), expectedResponse);
    BOOST_CHECK_EQUAL(reply->status(), expectedStatus);
}

// [0] - the initial url
// [1] - the expected finalUrl
std::tuple<const char*, const char*> redirectData[]
{
    std::tuple<const char*, const char*>
    {
        "http://google.com",
        "http://www.google.com/"
    },
    std::tuple<const char*, const char*>
    {
        "http://juot.net",
        "https://www.juot.net/forums/"
    },
    std::tuple<const char*, const char*>
    {
        "http://lulzapps.com",
        "https://www.owlclient.com/"
    },
//    std::tuple<const char*, const char*>
//    {
//        "http://www.washingtonpost.com/wp-srv/projects/yrreview/year.htm",
//        "http://www.washingtonpost.com/wp-srv/projects/yrreview/year.htm?noredirect=on"
//    }
};

BOOST_DATA_TEST_CASE(redirectTest, data::make(redirectData), url, expectedFinalUrl)
{
    owl::WebClient client;
    client.setThrowOnFail(false);
    owl::WebClient::ReplyPtr reply = client.GetUrl(QString::fromLatin1(url), owl::WebClient::NOTIDY | owl::WebClient::NOCACHE);
    
    BOOST_REQUIRE(reply != nullptr);
    BOOST_CHECK_EQUAL(reply->status(), 200);
    BOOST_CHECK_EQUAL(reply->finalUrl(), expectedFinalUrl);
}

// The SHA1 hashes are generated by viewing the source at
// https://codebeautify.org/source-code-viewer and then
// copying/pasting the text into the tool located at
// http://onlinemd5.com/. This selection of webpages were
// choosen because of the likelyhood that they will 
// never change.
std::tuple<const char*, const char*> webtextData[]
{
    std::tuple<const char*, const char*>
    {
        "http://www.cnn.com/US/OJ/",
        "67252B9BD74AB1080A508609496B88028C4403A6"
    },
    std::tuple<const char*, const char*>
    {
        "https://www.york.ac.uk/teaching/cws/wws/webpage1.html",
        "A3E86B5B146DCA332F862A8096158E009BEBAB23"
    },
//    std::tuple<const char*, const char*>
//    {
//        "http://web.ics.purdue.edu/~gchopra/class/public/pages/webdesign/05_simple.html",
//        "9F871718676BD6175EDE3EECAD1A9D137E81E842"
//    }
};

BOOST_DATA_TEST_CASE(webtextTest, data::make(webtextData), url, expectedhash)
{
    owl::WebClient client;
    client.setThrowOnFail(false);

    const QString rawtext = client.DownloadString(QString::fromStdString(url), owl::WebClient::NOTIDY);
    const QByteArray hash = QCryptographicHash::hash(rawtext.toUtf8(), QCryptographicHash::Sha1);
    const QString result = QString{ hash.toHex() }.toUpper();

    BOOST_CHECK_EQUAL(result.toStdString(), expectedhash);
}

// URL params
// POST params
// expected result
std::tuple<const char*, const char*, const char*> postGetData[]
{
    std::tuple<const char*, const char*, const char*>
    {
        "key1=val1&key2=val2",
        "",
        R"([g]~key1~ : ~val1~
[g]~key2~ : ~val2~
)"
    },
    std::tuple<const char*, const char*, const char*>
    {
        "",
        "key1=val1&key2=val2",
        R"([p]~key1~ : ~val1~
[p]~key2~ : ~val2~
)"
    },
    std::tuple<const char*, const char*, const char*>
    {
        "k1=v1",
        "key1=val1&key2=val2",
        R"([g]~k1~ : ~v1~
[p]~key1~ : ~val1~
[p]~key2~ : ~val2~
)"
    }
};

BOOST_DATA_TEST_CASE(postTest, data::make(postGetData), urlData, postData, expectedData)
{
    QString echoerUrl = R"(http://owlclient.com/tools/echoer.php)";

    if (auto params = QString::fromLatin1(urlData); !params.isEmpty())
    {
        echoerUrl += "?" + params;
    }

    owl::WebClient client;
    client.setThrowOnFail(false);
    owl::WebClient::ReplyPtr reply = client.PostUrl(echoerUrl, QString::fromStdString(postData), owl::WebClient::NOTIDY);

    BOOST_CHECK_EQUAL(reply->status(), 200);
    BOOST_CHECK_EQUAL(reply->text().toStdString(), expectedData);
}

std::tuple<const char*, const char*> rawData[]
{
    std::tuple<const char*, const char*>
    {
        "https://amb.la/images/amb-logo7.png",
        "B33F9B748EE1FF67954380DE76F3BDE8CEC91400"
    },
    std::tuple<const char*, const char*>
    {
        "https://i.imgur.com/c9m4pfC.jpg",
        "2EF53F7871CC673F69695013E4E5A28727F30CCA"
    },
    std::tuple<const char*, const char*>
    {
        "https://i.imgur.com/wsHc3pQ.png",
        "E8C04BE4737AA2F44C9ECB600EB6189BEC642030"
    }
};

BOOST_DATA_TEST_CASE(rawDataTest, data::make(rawData), url, expectedHash)
{
    owl::WebClient client;
    client.setThrowOnFail(false);

    auto reply = client.GetUrl(QString::fromLatin1(url), owl::WebClient::NOTIDY);

    QByteArray byteArray(reply->data().c_str(), 
        static_cast<uint>(reply->data().size()));
    const QByteArray hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha1);
    const QString result = QString{ hash.toHex() }.toUpper();

    BOOST_REQUIRE(reply != nullptr);
    BOOST_CHECK_EQUAL(reply->status(), 200);
    BOOST_CHECK_EQUAL(result.toStdString(), expectedHash);
}

BOOST_AUTO_TEST_SUITE_END()
