// Owl - www.owlclient.com
// Copyright (c) 2012-2018, Adalid Claure <aclaure@gmail.com>


#include <boost/test/unit_test.hpp>

#include <QtCore>
#include <iostream>

#include "../src/Utils/StringMap.h"

BOOST_AUTO_TEST_SUITE(StringMap)

BOOST_AUTO_TEST_CASE(StringMapTest)
{
    owl::StringMap params;

    params.add("constChar", "TEXT1");
    params.add("QString", QString("TEXT2"));
    params.add("int0", 0);
    params.add("int5", 5);
    params.add("boolTrue", true);

    BOOST_CHECK(params.getText("constChar") == "TEXT1");
    BOOST_CHECK(params.getText("constChar") != "badText");
    BOOST_CHECK(params.get<std::int32_t>("constChar", false) == 0);
    BOOST_CHECK(params.getBool("constChar", false) == false);

    BOOST_CHECK(params.getText("QString") == "TEXT2");
    BOOST_CHECK(params.getText("QString") != "badText");
    BOOST_CHECK(params.get<std::int32_t>("QString", false) == 0);
    BOOST_CHECK(params.getBool("QString", false) != true);

    BOOST_CHECK(params.getText("int0") != "badText");
    BOOST_CHECK(params.getText("int0") == "0");
    BOOST_CHECK_EQUAL(params.get<std::int32_t>("int0", false), 0);
    BOOST_CHECK_EQUAL(params.getBool("int0", false), false);

    BOOST_CHECK(params.getText("int5") != "badText");
    BOOST_CHECK(params.getText("int5") == "5");
    BOOST_CHECK(params.get<std::int32_t>("int5", false) != 0);
    BOOST_CHECK(params.get<std::int32_t>("int5", false) == 5);
    BOOST_CHECK(params.getBool("int5", false) == false);

    BOOST_CHECK(params.getText("boolTrue") != "badText");
    BOOST_CHECK(params.get<std::int32_t>("boolTrue", false) != 0);
    BOOST_CHECK(params.getBool("boolTrue", false) == true);

     BOOST_CHECK(params.has("int0"));
     params.erase("int0");
     BOOST_CHECK(!params.has("int0"));
     params.clear();
     BOOST_CHECK(!params.has("int5"));

     params.setOrAdd("boolTrue", false);
     params.setOrAdd("int5", 42);
     params.setOrAdd("constChar", "hello world");

     BOOST_CHECK(params.getText("constChar") == "hello world");
     BOOST_CHECK_EQUAL(params.get<std::int32_t>("int5"), 42);
     BOOST_CHECK_EQUAL(params.getBool("boolTrue"), false);
}

BOOST_AUTO_TEST_CASE(StringMapParseTest)
{
    const QString data1 = "animal=cat sound=meow";

    owl::StringMap map1;
    map1.parse(data1);

    BOOST_CHECK(map1.getText("animal") == "cat");
    BOOST_CHECK(map1.getText("sound") == "meow");

    const QString data2 = "animal=dog;sound=bark;food=steak";

    map1.clear();
    map1.parse(data2, ';');

    BOOST_CHECK(map1.getText("animal") == "dog");
    BOOST_CHECK(map1.getText("sound") == "bark");
    BOOST_CHECK(map1.getText("food") == "steak");

    const QString data3 = R"(
animal1=mouse;sound1=squeak;food1=cheese
animal2=cat;sound2=meow;food2=tuna
animal3=dog;sound3=bark;food3=steak)";

    map1.clear();
    map1.parseLines(data3, ';');

    BOOST_CHECK(map1.getText("animal1") == "mouse");
    BOOST_CHECK(map1.getText("sound1") == "squeak");
    BOOST_CHECK(map1.getText("food1") == "cheese");

    BOOST_CHECK(map1.getText("animal2") == "cat");
    BOOST_CHECK(map1.getText("sound2") == "meow");
    BOOST_CHECK(map1.getText("food2") == "tuna");

    BOOST_CHECK(map1.getText("animal3") == "dog");
    BOOST_CHECK(map1.getText("sound3") == "bark");
    BOOST_CHECK(map1.getText("food3") == "steak");
}

BOOST_AUTO_TEST_CASE(StringMapConstructTest)
{
    owl::StringMap params;

    params.add("constChar", "TEXT1");
    params.add("QString", QString("TEXT2"));
    params.add("int0", 0);
    params.add("int5", 5);
    params.add("boolTrue", true);

    // copy constructor
    owl::StringMap paramsCopy { params };

    BOOST_CHECK(params.getText("constChar") == "TEXT1");
    BOOST_CHECK(params.getText("QString") == "TEXT2");
    BOOST_CHECK_EQUAL(params.get<std::int32_t>("int0", false), 0);
    BOOST_CHECK(params.get<std::int32_t>("int5", false) ==  5);
    BOOST_CHECK(params.getBool("boolTrue", false) == true);

    BOOST_CHECK(paramsCopy.getText("constChar") == "TEXT1");
    BOOST_CHECK(paramsCopy.getText("QString") == "TEXT2");
    BOOST_CHECK_EQUAL(paramsCopy.get<std::int32_t>("int0", false), 0);
    BOOST_CHECK(paramsCopy.get<std::int32_t>("int5", false) ==  5);
    BOOST_CHECK(paramsCopy.getBool("boolTrue", false) == true);

    // move constructor
    owl::StringMap paramsMove { std::move(paramsCopy) };

    BOOST_CHECK(paramsMove.getText("constChar") == "TEXT1");
    BOOST_CHECK(paramsMove.getText("QString") == "TEXT2");
    BOOST_CHECK_EQUAL(paramsMove.get<std::int32_t>("int0", false), 0);
    BOOST_CHECK(paramsMove.get<std::int32_t>("int5", false) ==  5);
    BOOST_CHECK(paramsMove.getBool("boolTrue", false) == true);

    BOOST_CHECK_EQUAL(paramsCopy.size(), static_cast<std::size_t>(0));
}


BOOST_AUTO_TEST_SUITE_END()
