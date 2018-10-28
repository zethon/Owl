// Owl - www.owlclient.com
// Copyright (c) 2012-2018, Adalid Claure <aclaure@gmail.com>

#include <boost/test/unit_test.hpp>

#include <QtCore>

#include "../src/Utils/OwlUtils.h"

BOOST_AUTO_TEST_SUITE(OwlUtils)

BOOST_AUTO_TEST_CASE(testSanitizeUrl)
{
    BOOST_CHECK_EQUAL(owl::sanitizeUrl("http://www.juot.net").toStdString(), "http://www.juot.net");
    BOOST_CHECK_EQUAL(owl::sanitizeUrl("http://www.juot.net/").toStdString(), "http://www.juot.net");
    BOOST_CHECK_EQUAL(owl::sanitizeUrl("http://www.juot.net/index.php").toStdString(), "http://www.juot.net");
    BOOST_CHECK_EQUAL(owl::sanitizeUrl("http://www.juot.net/forums").toStdString(), "http://www.juot.net/forums");
    BOOST_CHECK_EQUAL(owl::sanitizeUrl("http://www.juot.net/forums/").toStdString(), "http://www.juot.net/forums");
    BOOST_CHECK_EQUAL(owl::sanitizeUrl("http://www.juot.net/forums/index.php").toStdString(), "http://www.juot.net/forums");
    BOOST_CHECK_EQUAL(owl::sanitizeUrl("http://www.juot.net/beta/forums").toStdString(), "http://www.juot.net/beta/forums");
    BOOST_CHECK_EQUAL(owl::sanitizeUrl("http://www.juot.net/beta/forums/").toStdString(), "http://www.juot.net/beta/forums");
    BOOST_CHECK_EQUAL(owl::sanitizeUrl("http://www.juot.net/beta/forums/index.php").toStdString(), "http://www.juot.net/beta/forums");

    BOOST_CHECK_EQUAL(owl::sanitizeUrl("www.juot.net").toStdString(), "http://www.juot.net");
    BOOST_CHECK_EQUAL(owl::sanitizeUrl("www.juot.net/").toStdString(), "http://www.juot.net");
    BOOST_CHECK_EQUAL(owl::sanitizeUrl("www.juot.net/forums").toStdString(), "http://www.juot.net/forums");
    BOOST_CHECK_EQUAL(owl::sanitizeUrl("www.juot.net/forums/").toStdString(), "http://www.juot.net/forums");
    BOOST_CHECK_EQUAL(owl::sanitizeUrl("www.juot.net/forums/index.php").toStdString(), "http://www.juot.net/forums");
}

BOOST_AUTO_TEST_SUITE_END()