#pragma once
#include <QtTest/QtTest>
#include <Parsers/BBCodeParser.h>
#include <Parsers/ParserManager.h>

using namespace owl;

class VBulletin3XTests: public QObject
{
	Q_OBJECT
    
    QString			_parserPath;
	ParserBasePtr	_parser;

public:
    VBulletin3XTests(const QString& parserPath);
    virtual ~VBulletin3XTests() = default;
    
private Q_SLOTS:
    // called by the framework befre the tests start
    void initTestCase();

    // generic parser tests
	void loadedvBulletin3xParser();
	
    // AMB specific tests
    void canParseAMB();
    void loginAMB();
	void testForumBrowsing();
	void testThreadBrowsing();
    
//	void AMBbadLogin();
//	void testAMB();
//	void testJUOT();
//	void testGAT();
//	void testBehindTheVoiceActors();
//	void testNR();
//	void testStringForum();
//	void testDetroitLionsForum();
//	void testMacRumors();
//	void testJeepsUnlimited();
//	void testGirlSoutWest();
//	void testArkansasCrawlers();		
//	void testPCGamer();
//	void testSEQAnswers();
//
//private Q_SLOTS:
//	void testParserCache();
//	//void testAMBUnread();


//private:
//	const static bool _bFullTest = true;
//
//#ifdef Q_OS_WIN
//	const QString _parserPath = "C:\\vbulletin\\deltas\\client\\parsers";
//#endif

};


//class ParserTests: public QObject
//{
//	Q_OBJECT
//
//private Q_SLOTS:
//	void toUpper();
//
//
//};

//class VBulletin3XTests: public QObject
//{
//	Q_OBJECT
//
//private:
////	void AMBbadLogin();
////	void testAMB();
////	void testJUOT();
////	void testGAT();
////	void testBehindTheVoiceActors();
////	void testNR();
////	void testStringForum();
////	void testDetroitLionsForum();
////	void testMacRumors();
////	void testJeepsUnlimited();
////	void testGirlSoutWest();
////	void testArkansasCrawlers();		
////	void testPCGamer();
////	void testSEQAnswers();
////
////private Q_SLOTS:
////	void testParserCache();
////	//void testAMBUnread();
//
//
//private:
//	const static bool _bFullTest = true;
//};
