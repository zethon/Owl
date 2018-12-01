#include <boost/test/unit_test.hpp>
#include "../src/Data/Board.h"

BOOST_AUTO_TEST_SUITE(Board)

BOOST_AUTO_TEST_CASE(VersionTest)
{
    owl::Board board("https://www.amb.la");
    BOOST_CHECK_EQUAL(board.getUrl().toStdString(), "https://www.amb.la");
}

BOOST_AUTO_TEST_SUITE_END()