// Owl - www.owlclient.com
// Copyright Adalid Claure <aclaure@gmail.com>

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <ostream>

#include "../src/Parsers/Tapatalk.h"

namespace data = boost::unit_test::data;

BOOST_AUTO_TEST_SUITE(Forum)

BOOST_AUTO_TEST_CASE(testTapatalk)
{
    owl::Tapatalk4x temp("https://xenforo.com/community/");
    BOOST_CHECK_EQUAL(temp.defaultPostsPerPage().first, 25);
}

BOOST_AUTO_TEST_SUITE_END()