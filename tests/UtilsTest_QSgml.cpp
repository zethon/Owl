// Owl - www.owlclient.com
// Copyright (c) 2012-2018, Adalid Claure <aclaure@gmail.com>

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <ostream>
#include <QtCore>

#include "../src/Utils/QSgml.h"
#include "../src/Utils/QSgmlTag.h"

namespace data = boost::unit_test::data;

BOOST_AUTO_TEST_SUITE(QSgmlTest)

std::tuple<std::string> htmlData[] = 
{
    { "<html><body>Hello World!</body></html>" },
};

BOOST_DATA_TEST_CASE(testParse, data::make(htmlData), htmlText)
{
    QSgml doc;
    BOOST_CHECK(doc.parse(QString::fromStdString(htmlText)));
}

BOOST_AUTO_TEST_SUITE_END()