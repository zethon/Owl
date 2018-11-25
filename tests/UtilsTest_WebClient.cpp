// Owl - www.owlclient.com
// Copyright (c) 2012-2018, Adalid Claure <aclaure@gmail.com>
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
    if (!spdlog::get("Owl"))
    {
        spdlog::stdout_color_mt("Owl")->set_level(spdlog::level::off);
    }

    owl::WebClient client;
    client.setThrowOnFail(false);
    auto reply = client.GetUrl(QString::fromLatin1(url), owl::WebClient::NOTIDY | owl::WebClient::NOCACHE);
    BOOST_REQUIRE(reply != nullptr);
    BOOST_CHECK_EQUAL(reply->data().toStdString(), expectedResponse);
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
        "http://amb.la",
        "https://amb.la/"
    },
    std::tuple<const char*, const char*>
    {
        "http://lulzapps.com",
        "http://www.owlclient.com/"
    }
};

BOOST_DATA_TEST_CASE(redirectTest, data::make(redirectData), url, expectedFinalUrl)
{
    if (!spdlog::get("Owl"))
    {
        spdlog::stdout_color_mt("Owl")->set_level(spdlog::level::off);
    }

    owl::WebClient client;
    client.setThrowOnFail(false);
    owl::WebClient::ReplyPtr reply = client.GetUrl(QString::fromLatin1(url), owl::WebClient::NOTIDY | owl::WebClient::NOCACHE);
    BOOST_CHECK_EQUAL(reply->finalUrl(), expectedFinalUrl);
}

std::tuple<const char*, const char*> webtextData[]
{
    std::tuple<const char*, const char*>
    {
        "http://www.cnn.com/US/OJ/",
        "2c9afb562185beeb1138b4676dc0f4b4cf1ead87"
    }
};

BOOST_DATA_TEST_CASE(webtextTest, data::make(webtextData), url, expectedhash)
{
    if (!spdlog::get("Owl"))
    {
        spdlog::stdout_color_mt("Owl")->set_level(spdlog::level::off);
    }

    owl::WebClient client;
    client.setThrowOnFail(false);

    QString rawtext = client.DownloadString(QString::fromStdString(url), owl::WebClient::NOTIDY);
    QFile file("/home/addy/Desktop/file.txt");
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );
        stream << rawtext;
    }
    file.close();

    QByteArray hash = QCryptographicHash::hash(rawtext.toUtf8(), QCryptographicHash::Sha1);

    QString result = hash.toHex();
    qDebug() << result;
//    56170f5429b35dea081bb659b884b475ca9329a9

}


BOOST_AUTO_TEST_SUITE_END()
