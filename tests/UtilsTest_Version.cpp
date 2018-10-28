#include <boost/test/unit_test.hpp>
#include "../src/Utils/Version.h"

BOOST_AUTO_TEST_SUITE(Version)

BOOST_AUTO_TEST_CASE(VersionTest)
{
    // owl::Version ver("3.5.1");

    // BOOST_CHECK(ver != owl::Version("3.5.0"));
    // BOOST_CHECK(ver != owl::Version("3.5"));
    // BOOST_CHECK(ver != owl::Version("3"));
    // BOOST_CHECK(ver != owl::Version("3.0.5"));
    // BOOST_CHECK(ver == owl::Version("3.5.1"));
    // BOOST_CHECK(ver != owl::Version("3.5.0"));
    // BOOST_CHECK(ver > owl::Version("3.5.0"));
    // BOOST_CHECK(ver > owl::Version("3.5.0"));
    // BOOST_CHECK(ver > owl::Version("3.5"));
    // BOOST_CHECK(ver > owl::Version("3.0"));
    // BOOST_CHECK(ver > owl::Version("3.0.0"));
    // BOOST_CHECK(ver > owl::Version("3"));
    // BOOST_CHECK(ver < owl::Version("4"));
    // BOOST_CHECK(ver < owl::Version("4.1"));
    // BOOST_CHECK(ver < owl::Version("4.1.1"));
    // BOOST_CHECK(ver > owl::Version("3.5"));
    // BOOST_CHECK(ver > owl::Version("3.0"));
}

BOOST_AUTO_TEST_CASE(VersionFailTest)
{
	// BOOST_CHECK_NO_THROW(owl::Version(""));
	// BOOST_CHECK_THROW(owl::Version("Goo"), std::runtime_error);
	// BOOST_CHECK_THROW(owl::Version("2.2.2.2.2.2"), std::runtime_error);
}


BOOST_AUTO_TEST_CASE(VersionToStringTest)
{
	// BOOST_CHECK_EQUAL(owl::Version("3.5.1").toString(), "3.5.1");
	// BOOST_CHECK_EQUAL(owl::Version("3.5.1").toString(true), "3.5.1");
	// BOOST_CHECK_EQUAL(owl::Version("3.5.1").toString(false), "3.5.1.0");

	// BOOST_CHECK_EQUAL(owl::Version("3.5.0.1").toString(), "3.5.0.1");
	// BOOST_CHECK_EQUAL(owl::Version("3.5.0.1").toString(true), "3.5.0.1");
	// BOOST_CHECK_EQUAL(owl::Version("3.5.0.1").toString(false), "3.5.0.1");

	// BOOST_CHECK_EQUAL(owl::Version("3.5.0.0").toString(), "3.5");
	// BOOST_CHECK_EQUAL(owl::Version("3.5.0.0").toString(true), "3.5");
	// BOOST_CHECK_EQUAL(owl::Version("3.5.0.0").toString(false), "3.5.0.0");
}

BOOST_AUTO_TEST_SUITE_END()