// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#include <QtTest/QtTest>

#include <Utils/DateTimeParser.h>
#include <Utils/Moment.h>
#include <Utils/OwlUtils.h>
#include <Utils/StringMap.h>
#include <Utils/Version.h>
#include "OwlUtilsTests.h"

void OwlUtilsTests::sanitizeUrlTest()
{
    QString input = "http://www.juot.net/";

    QCOMPARE(owl::sanitizeUrl("http://www.juot.net"), QString("http://www.juot.net"));
    QCOMPARE(owl::sanitizeUrl("http://www.juot.net/"), QString("http://www.juot.net"));
    QCOMPARE(owl::sanitizeUrl("http://www.juot.net/index.php"), QString("http://www.juot.net"));
    QCOMPARE(owl::sanitizeUrl("http://www.juot.net/forums"), QString("http://www.juot.net/forums"));
    QCOMPARE(owl::sanitizeUrl("http://www.juot.net/forums/"), QString("http://www.juot.net/forums"));
    QCOMPARE(owl::sanitizeUrl("http://www.juot.net/forums/index.php"), QString("http://www.juot.net/forums"));
    QCOMPARE(owl::sanitizeUrl("http://www.juot.net/beta/forums"), QString("http://www.juot.net/beta/forums"));
    QCOMPARE(owl::sanitizeUrl("http://www.juot.net/beta/forums/"), QString("http://www.juot.net/beta/forums"));
    QCOMPARE(owl::sanitizeUrl("http://www.juot.net/beta/forums/index.php"), QString("http://www.juot.net/beta/forums"));

    QCOMPARE(owl::sanitizeUrl("www.juot.net"), QString("http://www.juot.net"));
    QCOMPARE(owl::sanitizeUrl("www.juot.net/"), QString("http://www.juot.net"));
    QCOMPARE(owl::sanitizeUrl("www.juot.net/forums"), QString("http://www.juot.net/forums"));
    QCOMPARE(owl::sanitizeUrl("www.juot.net/forums/"), QString("http://www.juot.net/forums"));
    QCOMPARE(owl::sanitizeUrl("www.juot.net/forums/index.php"), QString("http://www.juot.net/forums"));
}

void OwlUtilsTests::versionTest()
{
    owl::Version ver("3.5.1");

    QCOMPARE(ver == owl::Version("3.5.0"), false);
    QCOMPARE(ver == owl::Version("3.5"), false);
    QCOMPARE(ver == owl::Version("3"), false);
    QCOMPARE(ver == owl::Version("3.0.5"), false);
    QCOMPARE(ver == owl::Version("3.5.1"), true);
    QCOMPARE(ver != owl::Version("3.5.0"), true);
    QCOMPARE(ver > owl::Version("3.5.0"), true);
    QCOMPARE(ver < owl::Version("3.5.0"), false);
    QCOMPARE(ver > owl::Version("3.5"), true);
    QCOMPARE(ver > owl::Version("3.0"), true);
    QCOMPARE(ver > owl::Version("3.0.0"), true);
    QCOMPARE(ver > owl::Version("3"), true);
    QCOMPARE(ver < owl::Version("4"), true);
    QCOMPARE(ver < owl::Version("4.1"), true);
    QCOMPARE(ver < owl::Version("4.1.1"), true);
    QCOMPARE(ver > owl::Version("3.5"), true);
    QCOMPARE(ver > owl::Version("3.0"), true);
}

void OwlUtilsTests::StringMapTest()
{
    owl::StringMap params;

    params.add("constChar", "TEXT1");
    params.add("QString", QString("TEXT2"));
    params.add("int0", 0);
    params.add("int5", 5);
    params.add("boolTrue", true);

    QCOMPARE(params.getText("constChar") == "TEXT1", true);
    QCOMPARE(params.getText("constChar") == "badText", false);
    QCOMPARE(params.getInt("constChar", false) == 0, true);
    QCOMPARE(params.getBool("constChar", false) == false, true);

    QCOMPARE(params.getText("QString") == "TEXT2", true);
    QCOMPARE(params.getText("QString") == "badText", false);
    QCOMPARE(params.getInt("QString", false) == 0, true);
    QCOMPARE(params.getBool("QString", false) == true, false);

    QCOMPARE(params.getText("int0") == "badText", false);
    QCOMPARE(params.getText("int0") == "0", true);
    QCOMPARE(params.getInt("int0", false), 0);
    QCOMPARE(params.getBool("int0", false) == true, false);

    QCOMPARE(params.getText("int5") == "badText", false);
    QCOMPARE(params.getText("int5") == "5", true);
    QCOMPARE(params.getInt("int5", false) ==  0, false);
    QCOMPARE(params.getInt("int5", false) ==  5, true);
    QCOMPARE(params.getBool("int5", false) == false, true);

    QCOMPARE(params.getText("boolTrue") == "badText", false);
    QCOMPARE(params.getInt("boolTrue", false) == 0, false);
    QCOMPARE(params.getBool("boolTrue", false) == true, true);

    QCOMPARE(params.has("int0"), true);
    params.erase("int0");
    QCOMPARE(params.has("int0"), false);
    params.clear();
    QCOMPARE(params.has("int5"), false);
}

void OwlUtilsTests::MomentTest()
{
    QDateTime fakenow(QDate(2016,8,23), QTime(5,30));

    owl::Moment m;

    m.setDateTime(QDateTime(QDate(2016,8,23), QTime(5,30)));
    QCOMPARE(m.toString(fakenow), QStringLiteral("A few seconds ago"));

    m.setDateTime(QDateTime(QDate(2016,8,23), QTime(5,29)));
    QCOMPARE(m.toString(fakenow), QStringLiteral("A minute ago"));

    m.setDateTime(QDateTime(QDate(2016,8,23), QTime(5,01)));
    QCOMPARE(m.toString(fakenow), QStringLiteral("29 minutes ago"));

    m.setDateTime(QDateTime(QDate(2016,8,23), QTime(4,30)));
    QCOMPARE(m.toString(fakenow), QStringLiteral("An hour ago"));

    m.setDateTime(QDateTime(QDate(2016,8,23), QTime(2,45)));
    QCOMPARE(m.toString(fakenow), QStringLiteral("2 hours ago"));

    m.setDateTime(QDateTime(QDate(2016,8,22), QTime(05,30)));
    QCOMPARE(m.toString(fakenow), QStringLiteral("A day ago"));

    m.setDateTime(QDateTime(QDate(2016,8,13), QTime(5,29)));
    QCOMPARE(m.toString(fakenow), QStringLiteral("10 days ago"));

    m.setDateTime(QDateTime(QDate(2016,7,23), QTime(5,29)));
    QCOMPARE(m.toString(fakenow), QStringLiteral("A month ago"));

    m.setDateTime(QDateTime(QDate(2016,6,23), QTime(5,29)));
    QCOMPARE(m.toString(fakenow), QStringLiteral("2 months ago"));

    m.setDateTime(QDateTime(QDate(2016,3,23), QTime(5,29)));
    QCOMPARE(m.toString(fakenow), QStringLiteral("5 months ago"));

    m.setDateTime(QDateTime(QDate(2015,12,23), QTime(5,29)));
    QCOMPARE(m.toString(fakenow), QStringLiteral("8 months ago"));

    m.setDateTime(QDateTime(QDate(2015,5,23), QTime(5,29)));
    QCOMPARE(m.toString(fakenow), QStringLiteral("A year ago"));

    m.setDateTime(QDateTime(QDate(2013,5,23), QTime(5,29)));
    QCOMPARE(m.toString(fakenow), QStringLiteral("3 years ago"));
}

void OwlUtilsTests::PreviewTextTest()
{
    const QString shortText { "Lorem ipsum dolor sit amet, consectetur." };
//    QCOMPARE(owl::previewText(shortText), QString{"Lorem ipsum dolor sit amet, consectetur."});
    QCOMPARE(owl::previewText(shortText, 20), QString{"Lorem ipsum dolor..."});

//    const QString longText { "Lorem ipsum dolor sit amet,   consectetur."};
//    const QString longText { "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."};
}
