// Owl - www.owlclient.com
// Copyright Adalid Claure <aclaure@gmail.com>

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <ostream>

#include "../src/Parsers/BBCodeParser.h"

namespace data = boost::unit_test::data;

BOOST_AUTO_TEST_SUITE(BBcodeParser)

std::tuple<const char*, const char*> bbcodeData[] = 
{
    std::tuple<const char*, const char*>
    { 
        "Hello World!", "Hello World!"
    },
    std::tuple<const char*, const char*>
    { 
        "Hello [b]World[/b]!", "Hello <b>World</b>!"
    },
};

BOOST_DATA_TEST_CASE(testParse, data::make(bbcodeData), testText, expected)
{
    owl::BBRegExParser parser;

    parser.resetQuoteStyle();
   
   auto temp = parser.toHtml(QString::fromStdString(testText));
   BOOST_CHECK_EQUAL(temp.toStdString(), expected);
}

BOOST_AUTO_TEST_SUITE_END()