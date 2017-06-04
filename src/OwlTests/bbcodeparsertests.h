#pragma once
#include <QtTest/QtTest>
#include <Parsers/BBCodeParser.h>

using namespace owl;

class BBRegExParserTests : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void VBQuoteTests();
    void simpleTagTests();
    void simpleQuoteTests();
    void toTextTests();
//    void KeyValueQuoteTests();
};

//using TextTestPair = std::pair<quint32, QString>;
//using TextTestPairList = std::vector<TextTestPair>;

//using SimpleTagTuple = std::tuple<quint32, QString, bool, bool>;
//using SimpleTagTupleList = std::vector<SimpleTagTuple>;

//struct SimpleTagTestSet
//{
//    QString text;
//    TextTestPairList textlist;
//    SimpleTagTupleList taglist;
//};

//using SimpleTagTestSetList = std::vector<SimpleTagTestSet>;

// index in the token list, tag name, value, mathed?, closing tag?
//using ValueTagTuple = std::tuple<quint32, QString, QString, bool, bool>;
//using ValueTagTupleList = std::vector<ValueTagTuple>;

//struct ValueTagSet
//{
//    QString text;
//    TextTestPairList    textlist;
//    SimpleTagTupleList  simpletags;
//    ValueTagTupleList   valuetags;
//};

//using ValueTagSetList = std::vector<ValueTagSet>;

//class BBCodeParserTests : public QObject
//{
//    Q_OBJECT

//    void testTextNodes(const BBNodeList& nodelist, const TextTestPairList& textlist) const;
//    void testSimpleTags(const BBNodeList& nodelist, const SimpleTagTupleList& taglist) const;
//    void testValueTags(const BBNodeList& nodelist, const ValueTagTupleList& taglist) const;

//private Q_SLOTS:
//    void scannerTest();

//    void simpleTagParserTest();
//    void valueTagParserTest();

//};
