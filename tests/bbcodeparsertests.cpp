#include "BBCodeParserTests.h"

using namespace owl;

const std::vector<std::pair<QString,QString>> simpleTests
{
    { "[b]hi there[/b]", "<b>hi there</b>" },
    { "This [b][i]IS A TEST[/b][/i] of [b] something [i]cool[/i] ok? [/i]",
      "This <b><i>IS A TEST</b></i> of [b] something <i>cool</i> ok? [/i]" },
    { "This is an image: [img]http://www.google.com/image/gif[/img] What do you [b]think[/b]?",
      "This is an image: <img src=\"http://www.google.com/image/gif\" onload=\"NcodeImageResizer.createOn(this);\" /> What do you <b>think</b>?" },
    { "Check out [url=http://www.google.com]this url![/url] It's [b]awesome![/b]",
      "Check out <a href=\"http://www.google.com\">this url!</a> It's <b>awesome!</b>" }
};

void BBRegExParserTests::simpleTagTests()
{
    BBRegExParser parser;
    parser.setQuoteStyle(BBRegExParser::QuoteStyle::VBULLETIN);

    for (const auto& st : simpleTests)
    {
//        qDebug() << "Current Test: " << st.first;
        QCOMPARE(parser.toHtml(st.first), st.second);
    }
}

const std::vector<std::pair<QString,QString>> toTextTestStrings
{
    { "Check out [url=http://www.google.com]this url![/url] It's [b]awesome![/b]",
        "Check out this url! It's awesome!" },

    { "[b]hi there[/b]",
        "hi there" },

    { "This [b][i]IS A TEST[/b][/i] of [b] something [i]cool[/i] ok? [/i]",
        "This IS A TEST of  something cool ok? " },

    { "[QUOTE]This is quoted text[/QUOTE]Your text sucks.",
        "> This is quoted text\n\nYour text sucks." },

    { "[QUOTE=Kevin Hoffman;556233]This is some quoted text[/QUOTE]This is a response to quote text.",
        "Kevin Hoffman wrote:\n> This is some quoted text\n\nThis is a response to quote text." },
};

void BBRegExParserTests::toTextTests()
{
    BBRegExParser parser;
    parser.setQuoteStyle(BBRegExParser::QuoteStyle::UNKNOWN);

    for (const auto& st : toTextTestStrings)
    {
        qDebug() << "Current Test: " << st.first;
        QCOMPARE(parser.toPlainText(st.first), st.second);
    }
}

const std::vector<std::pair<QString, QString>> simpleQuoteTestStrings
{
    { "[QUOTE][url=http://www.anothermessageboard.com/showthread.php?p=123456]Originally Posted by Max Power[/url][/QUOTE]This is the post.",
        "<blockquote><b>Max Power</b> wrote:<br/></blockquote>This is the post."},

    { "Have you seen this: [QUOTE]This is a quote - George Washington[/QUOTE]I mean, wow!",
        "Have you seen this: <blockquote>This is a quote - George Washington</blockquote>I mean, wow!" },

    { "[QUOTE]This is a quote - [QUOTE]Long live and be happy! - Spock's cousin[/QUOTE] - George Washington[/QUOTE]I mean, wow!",
        "<blockquote>This is a quote - <blockquote>Long live and be happy! - Spock's cousin</blockquote> - George Washington</blockquote>I mean, wow!" }
};

void BBRegExParserTests::simpleQuoteTests()
{
    BBRegExParser parser;

    for (const auto& st : simpleQuoteTestStrings)
    {
//        qDebug() << "Current Test: " << st.first;
        QCOMPARE(parser.toHtml(st.first), st.second);
    }
}

const std::vector<std::pair<QString,QString>> VBquoteTests
{
    { "[QUOTE=Max Power;12345]Quote test![/QUOTE]Hi!",
      "<blockquote><b>Max Power</b> wrote:<br/><br/>Quote test!</blockquote>Hi!" },

    { "[QUOTE=Max Power;12345]Quote test![/QUOTE]This is a xx quote text!",
      "<blockquote><b>Max Power</b> wrote:<br/><br/>Quote test!</blockquote>This is a xx quote text!" },

    { "[QUOTE=Kevin Hoffman;556233]This is some quoted text[/QUOTE]This is a response to quote text.",
      "<blockquote><b>Kevin Hoffman</b> wrote:<br/><br/>This is some quoted text</blockquote>This is a response to quote text." },

    { "[QUOTE=Kevin Hoffman;556233]This is[QUOTE=adad;123] some [/QUOTE][/QUOTE][/QUOTE][/QUOTE]quoted text[/QUOTE]Hi!",
      "<blockquote><b>Kevin Hoffman</b> wrote:<br/><br/>This is<blockquote><b>adad</b> wrote:<br/><br/> some [/QUOTE][/QUOTE][/QUOTE]</blockquote>quoted text</blockquote>Hi!" },

    { "[QUOTE=Max Power   ; 123             ]OK![/ QUOTE ]",
        "[QUOTE=Max Power   ; 123             ]OK![/ QUOTE ]" },

    { "[QUOTE=Fÿnn;123]What were you thinking?[/QUOTE]Hello!",
      "<blockquote><b>Fÿnn</b> wrote:<br/><br/>What were you thinking?</blockquote>Hello!" },
};


// This functionality will test one v
void BBRegExParserTests::VBQuoteTests()
{
    BBRegExParser parser;

    for (const auto& st : VBquoteTests)
    {
//        qDebug() << "Current Test: " << st.first;
        QCOMPARE(parser.toHtml(st.first), st.second);
    }
}

////[quote uid=17178 name="Fÿn" post=1118275] That video from LaDouche is part of a bigger more than one hour video he did based on the scripts of the alummni of some art school. [/QUOTE] "Art Schools" are so bad that they didn't even allow Hitler to attend.

//void BBRegExParserTests::KeyValueQuoteTests()
//{
//    BBRegExParser parser;
//    parser.setQuoteStyle(BBRegExParser::QuoteStyle::KEYVALUE);
//}



// ****************************************************************************************************************************** //
//void BBCodeParserTests::testTextNodes(const BBNodeList& nodelist, const TextTestPairList& textlist) const
//{
//    for (const auto& pair : textlist)
//    {
//        const auto& idx = pair.first;
//        QVERIFY(nodelist.at(idx) != nullptr);
//        QCOMPARE(nodelist[idx]->isTag(), false);
//        QCOMPARE(nodelist[idx]->getText(), pair.second);
//    }
//}

//void BBCodeParserTests::testSimpleTags(const BBNodeList& nodelist, const SimpleTagTupleList& taglist) const
//{
//    for (const auto& vtuple : taglist)
//    {
//        quint32 idx;
//        QString tagname;
//        bool matched;
//        bool closing;

//        std::tie(idx, tagname, matched, closing) = vtuple;
//        const auto tag = std::dynamic_pointer_cast<BBTag>(nodelist[idx]);

//        QVERIFY(tag != nullptr);
//        QVERIFY(tag->getText() == tagname);
//        QVERIFY((tag->getMatchedTag() != nullptr) == matched);
//        QVERIFY(tag->isClosingTag() == closing);
//    }
//}

//void BBCodeParserTests::testValueTags(const BBNodeList& nodelist, const ValueTagTupleList& taglist) const
//{
//    for (const auto& vtuple : taglist)
//    {
//        quint32 idx;
//        QString tagname;
//        QString value;
//        bool matched;
//        bool closing;

//        std::tie(idx, tagname, value, matched, closing) = vtuple;
//        const auto tag = std::dynamic_pointer_cast<BBValueTag>(nodelist[idx]);

//        QVERIFY(tag != nullptr);
//        QVERIFY(tag->getText() == tagname);
//        QVERIFY(tag->getValue() == value);
//        QVERIFY((tag->getMatchedTag() != nullptr) == matched);
//        QVERIFY(tag->isClosingTag() == closing);
//    }
//}

/////
///// \brief BBCodeParserTests::scannerTest
/////
//void BBCodeParserTests::scannerTest()
//{
//    //QString _bbcode1 = "aa[url=http://www.google.com][b]cc[/b]dd[/url]zzzz";

////    QString text1 = "[b]Hello world![/b]";
////    QTextStream textStream1(&text1);

////    owl::BBCScanner scanner;
////    const auto tokens1 = scanner.Scan(textStream1);

////    QCOMPARE(tokens1.size(), 0);

//}

//// First member is the string to be parsed
//// Second is an array indicies and text of the text items
//// Third is an array of the tags, with the args being:
////      index, tag name, matched?, closing?
//const SimpleTagTestSetList tests =
//{
//    {
//        "[",
//        {
//            { 0u, QString("[") }
//        },
//        {}
//    },

//    {
//        "[/b",
//        {
//            { 0u, QString("[/b") }
//        },
//        {}
//    },

//    {
//        "]",
//        {
//            { 0u, QString("]") }
//        },
//        {}
//    },

//    {
//        "111[b]33333[b]44444[/b]5555[b]6666[/b]EnD",
//        {  { 0u , QString("111") },
//           { 2u , QString("33333") },
//           { 4u , QString("44444") },
//           { 6u , QString("5555") },
//           { 8u , QString("6666") },
//           { 10u , QString("EnD") },
//        },
//        {
//            {1u, QString("b"), false, false},
//            {3u, QString("b"), true, false},
//            {5u, QString("b"), true, true},
//            {7u, QString("b"), true, false},
//            {9u, QString("b"), true, true}
//        }
//    },

//    {
//        "[b][img][b][/img][b][b][/b]",
//        {},
//        {
//            {0u, QString("b"), false, false},
//            {1u, QString("img"), true, false},
//            {2u, QString("b"), false, false},
//            {3u, QString("img"), true, true},
//            {4u, QString("b"), false, false},
//            {5u, QString("b"), true, false},
//            {6u, QString("b"), true, true},
//        }
//    },

//    {
//        "This is [ b ] a bb code [/b ] example![/url]",
//        {
//            { 0u, QString("This is [ b ] a bb code [/b ] example!") }
//        },
//        {
//            { 1u, QString("url"), false, true},
//        }
//    },

//    {
//        "[[[[[[b]]]]]]]]][/b[/[",
//        {
//            { 0u, QString("[[[[[") },
//            { 2u, QString("]]]]]]]][/b[/[") }
//        },
//        {
//            { 1u, QString("b"), false, false},
//        }
//    },

//    {
//        "Foo[/[i/[i]middle[ /i][/i]bar[i]",
//        {
//            { 0u, QString("Foo[/[i/") },
//            { 2u, QString("middle[ /i]") },
//            { 4u, QString("bar") }
//        },
//        {
//            { 1u, QString("i"), true, false},
//            { 3u, QString("i"), true, true},
//            { 5u, QString("i"), false, false},
//        }
//    }
//};

/////
///// \brief BBCodeParserTests::simpleTagParserTest
/////
//void BBCodeParserTests::simpleTagParserTest()
//{
//    for (const auto& t : tests)
//    {
//        QString bbcode(t.text);
//        qDebug() << "Test string: [" << bbcode << "]";

//        BBScanner scanner;
//        auto tokens = scanner.Scan(&bbcode);

//        BBParser parser(tokens); // copy on purpose
//        QCOMPARE(parser.parse(false), true);

//        const auto nodelist = parser.getNodeList();
//        QVERIFY(nodelist.size() > 0);

//        testTextNodes(nodelist, t.textlist);
//        testSimpleTags(nodelist, t.taglist);
//    }
//}

//const ValueTagSetList valueTestData =
//{
//    {
//        "[b][quote=bob][/b]",
//        {},
//        {
//            {0u, QString("b"), true, false},
//            {2u, QString("b"), true, true}
//        },
//        {
//            {1u, QString("quote"), QString("bob"), false, false},
//        }
//    },
//    {
//        "[b][quote=bob[/b]",
//        {
//            {1u, QString("[quote=bob")}
//        },
//        {
//            {0u, QString("b"), true, false},
//            {2u, QString("b"), true, true}
//        },
//        {}
//    },
//    {
//        "[b][quote=bob",
//        {
//            {1u, QString("[quote=bob")}
//        },
//        {
//            {0u, QString("b"), false, false},
//        },
//        {}
//    },
//    {
//        "[/[url=sometext]]to be url'd![/url[/[b]bold[/b][img][/url][foo=bar]foobar!",
//        {
//            { 0u, QString("[/") },
//            { 2u, QString("]to be url'd![/url[/") },
//            { 4u, QString("bold") },
//            { 9u, QString("foobar!") }
//        },
//        {
//            { 3u, QString("b"), true, false },
//            { 5u, QString("b"), true, true },
//            { 6u, QString("img"), false, false },
//            { 7u, QString("url"), true, true },
//        },
//        {
//            { 1u, QString("url"), QString("sometext"), true, false },
//            { 8u, QString("foo"), QString("bar"), false, false },
//        }
//    }
//};

/////
///// \brief BBCodeParserTests::valueTagParserTest
/////
//void BBCodeParserTests::valueTagParserTest()
//{
//    for (const auto& t : valueTestData)
//    {
//        QString bbcode(t.text);
//        qDebug() << "Test string: [" << bbcode << "]";

//        BBScanner scanner;
//        auto tokens = scanner.Scan(&bbcode);

//        BBParser parser(tokens); // copy on purpose
//        QCOMPARE(parser.parse(false), true);

//        const auto nodelist = parser.getNodeList();
//        QVERIFY(nodelist.size() > 0);

//        testTextNodes(nodelist, t.textlist);
//        testSimpleTags(nodelist, t.simpletags);
//        testValueTags(nodelist, t.valuetags);
//    }
//}
