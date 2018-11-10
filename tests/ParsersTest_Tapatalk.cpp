// Owl - www.owlclient.com
// Copyright Adalid Claure <aclaure@gmail.com>

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <ostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "../src/Parsers/Tapatalk.h"

namespace data = boost::unit_test::data;

BOOST_AUTO_TEST_SUITE(Forum)

BOOST_AUTO_TEST_CASE(testTapatalk)
{
    if (!spdlog::get("Owl"))
    {
        spdlog::stdout_color_mt("Owl")->set_level(spdlog::level::off);
    }

    owl::Tapatalk4x temp("https://xenforo.com/community/");
    BOOST_CHECK_EQUAL(temp.defaultPostsPerPage().first, static_cast<uint>(25));
}

BOOST_AUTO_TEST_SUITE_END()
