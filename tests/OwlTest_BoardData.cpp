#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include "../src/Data/Board.h"
#include "../src/Data/BoardManager.h"

BOOST_AUTO_TEST_SUITE(Board)

BOOST_AUTO_TEST_CASE(simpleBoardTest)
{
    owl::Board board("https://www.amb.la");
    board.setUsername("username");
    board.setPassword("password");
    board.setName("AMB");

    BOOST_CHECK_EQUAL(board.getUrl().toStdString(), "https://www.amb.la");
    BOOST_CHECK_EQUAL(board.getUsername().toStdString(), "username");
    BOOST_CHECK_EQUAL(board.getPassword().toStdString(), "password");
    BOOST_CHECK_EQUAL(board.getName().toStdString(), "AMB");
}

BOOST_AUTO_TEST_CASE(simpleBoardManagerTest)
{
    const auto temp_db = boost::filesystem::temp_directory_path() 
        / boost::filesystem::unique_path("owltest%%%%%%") / "owl.sqlite";

    const QString dbFilename{ QString::fromStdString(temp_db.string()) };

    auto db = owl::BoardManager::instance()->initializeDatabase(dbFilename);
    
    BOOST_REQUIRE(QFileInfo{ dbFilename }.exists());
    BOOST_REQUIRE(db.isValid());
    BOOST_REQUIRE(db.isOpen());
}

BOOST_AUTO_TEST_SUITE_END()