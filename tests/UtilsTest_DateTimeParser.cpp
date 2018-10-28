// Owl - www.owlclient.com
// Copyright (c) 2012-2018, Adalid Claure <aclaure@gmail.com>

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <ostream>
#include <QtCore>

#include "../Utils/DateTimeParser.h"

namespace std
{

std::ostream& operator<<(std::ostream& out, const QDate& qdate)
{
    return out << qdate.toString().toStdString();
}

std::ostream& operator<<(std::ostream& out, const QTime& qtime)
{
    return out << qtime.toString().toStdString();
}

}

BOOST_AUTO_TEST_SUITE(DateTimeParser)

BOOST_AUTO_TEST_CASE(DateParseTest)
{
    owl::DateTimeParser dtp;

    BOOST_CHECK_EQUAL(dtp.parseDate("03-24-1995"), QDate(1995,3,24));
}

BOOST_AUTO_TEST_CASE(TimeParseTest)
{
    owl::DateTimeParser dtp;

    BOOST_CHECK_EQUAL(dtp.parseTime("03:22"), QTime(3,22));
}

BOOST_AUTO_TEST_SUITE_END()