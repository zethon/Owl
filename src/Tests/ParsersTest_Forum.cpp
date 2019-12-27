// Owl - www.owlclient.com
// Copyright Adalid Claure <aclaure@gmail.com>

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <ostream>

#include "../src/Parsers/Forum.h"

namespace data = boost::unit_test::data;

BOOST_AUTO_TEST_SUITE(Forum)

BOOST_AUTO_TEST_CASE(testForum)
{
    owl::Forum f;
    f.setName("General");
    BOOST_CHECK_EQUAL(f.getName().toStdString(), "General");
}

BOOST_AUTO_TEST_SUITE_END()