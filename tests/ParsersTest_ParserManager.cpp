// Owl - www.owlclient.com
// Copyright Adalid Claure <aclaure@gmail.com>

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <ostream>

#include "../src/Parsers/ParserManager.h"

namespace data = boost::unit_test::data;

BOOST_AUTO_TEST_SUITE(ParserManager)

BOOST_AUTO_TEST_CASE(testParserManager)
{
    auto pm = owl::ParserManager::instance();
    BOOST_CHECK_EQUAL(pm->getParserTypeCount(), static_cast<std::size_t>(0));
}

BOOST_AUTO_TEST_SUITE_END()