#include <QtTest/QtTest>
#include "ParserTests.h"


VBulletin3XTests::VBulletin3XTests(const QString& parserPath)
    : _parserPath(parserPath)
{
    // do nothing
}

void VBulletin3XTests::initTestCase()
{
	QVERIFY(!_parserPath.isEmpty());
	
	QDir parserDir(_parserPath);
	QVERIFY(parserDir.exists());

    PARSERMGR->init(true, _parserPath);
}

void VBulletin3XTests::loadedvBulletin3xParser()
{
	auto names = PARSERMGR->getParserNames();
	auto vbIdx = names.indexOf("vbulletin3x");

	QVERIFY(vbIdx != -1);
}

void VBulletin3XTests::canParseAMB()
{
//	// test the long url first
//	const QString ambUrl = "http://www.anothermessageboard.com";
//	const QString ambShortUrl = "http://www.amb.la";

//	WebClient client;
//	QString html = client.DownloadString(ambUrl, RequestEnums::NOCACHE);
//	QVERIFY(!html.isEmpty());

//	// test out the full url
//	ParserBasePtr parser = PARSERMGR->createParser("vbulletin3x", ambUrl);
//	QVERIFY(parser.get() != nullptr);
//	QVERIFY(parser->canParse(html));

//	// test out the short url
//	parser = PARSERMGR->createParser("vbulletin3x", ambUrl);
//	QVERIFY(parser.get() != nullptr);
//	QVERIFY(parser->canParse(html));

//	// reset the variables we're using
//	html.clear();
//	parser.reset();

//	// download front page from the the short url
//	html = client.DownloadString(ambShortUrl, RequestEnums::NOCACHE);
//	QVERIFY(!html.isEmpty());

//	// test out the full url
//	parser = PARSERMGR->createParser("vbulletin3x", ambShortUrl);
//	QVERIFY(parser.get() != nullptr);
//	QVERIFY(parser->canParse(html));
}

void VBulletin3XTests::loginAMB()
{
	const QString ambUrl = "http://www.anothermessageboard.com";
    
	_parser.reset();
	_parser = PARSERMGR->createParser("vbulletin3x", ambUrl);

	// test a bad login
	LoginInfo badInfo("Foo", "Bar");
	auto badResults = _parser->login(badInfo);

	// A bad login should return two elements, the 'success' boolean set to false, and
	// the 'error' element containing the text explanation of the error
	QVERIFY(badResults.size() == 2);
	QVERIFY(badResults.has("error"));
	QVERIFY(!badResults.getText("error").isEmpty());
	QVERIFY(badResults.has("success"));
	QVERIFY(!badResults.getBool("success"));

	// test a valid login
    LoginInfo info("TestUser", "password");
	auto results = _parser->login(info);

    QVERIFY(results.has("success"));
    QVERIFY(results.getBool("success"));
}

void VBulletin3XTests::testForumBrowsing()
{
	// get the root forum list of sub-forums
	auto rootId = _parser->getRootForumId();
	auto forumList = _parser->getForumList(rootId);
	
	// AMB's root forum has 2 sub-forums
	QCOMPARE(forumList.size(), 2);

	// verify the names of the first two sub-forums
	QCOMPARE(forumList.at(0)->getName(), QString("General Topics"));
	QCOMPARE(forumList.at(1)->getName(), QString("AMB After Dark"));

	// Get sub-forums of Mike&R's Forum
	forumList = _parser->getForumList("76");
	QCOMPARE(forumList.size(), 3);

	// verify the sub-forum names
	QCOMPARE(forumList.at(0)->getName(), QString("DrHughGRection's Forum"));
	QCOMPARE(forumList.at(1)->getName(), QString("Skull Island"));
	QCOMPARE(forumList.at(2)->getName(), QString("Sex, Sex, Sex"));
	
	forumList = _parser->getForumList("BADID");
	QCOMPARE(forumList.size(), 0);
}

void VBulletin3XTests::testThreadBrowsing()
{
	// get the threads of the "General" forum
	auto threadList = _parser->getThreadList(ForumPtr(new Forum("2")));
	QVERIFY(threadList.size() > 0);

	// get the threads of the "News & Current Events" forum
	threadList = _parser->getThreadList(ForumPtr(new Forum("3")));
	QVERIFY(threadList.size() > 0);

	// try to get the threadList of a bad forum
	threadList = _parser->getThreadList(ForumPtr(new Forum("BADID")));
	QVERIFY(threadList.size() == 0);

	// Poke at the "General" forum
//	auto oldestThreads = _
}

//void VBulletin3XTests::testParserCache()
//{
//	VBulletin3X parser("http://www.anothermessageboard.com");
//	StringMap	results = parser.login(loginInfo);
//
//	ForumList list1 = parser.getForumList("-1");
//	ForumList list2 = parser.getForumList("-1");
//}

//void VBulletin3XTests::AMBbadLogin()
//{
//	if (!_bFullTest)
//	{
//		QSKIP("Skipping....", SkipSingle);
//	}
//
//	VBulletin3X parser("http://www.anothermessageboard.com");
//	LoginInfo loginInfo("TestUser", "BADPASSWORD");
//	StringMap results;
//	
//	QBENCHMARK_ONCE
//	{
//		results = parser.login(loginInfo);
//	}
//
//	QCOMPARE(results.has("success"), true);
//	QCOMPARE(results.getBool("success"), false);
//	QCOMPARE(results.has("securityToken"), false);
//	QCOMPARE(results.has("sessionKey"), false);
//}

////void VBulletin3XTests::testAMBUnread()
////{
////	BoardPtr board(new Board("http://www.anothermessageboard.com"));
////	board->setParser(ParserBasePtr(new VBulletin3X(board->getUrl())));
////
////	StringMap	results = board->getParser()->login(loginInfo);
////
////	ForumList list = board->getParser()->getUnreadForums();
////
////	qDebug() << "list size: " << list.size();
////}
//
//void VBulletin3XTests::testAMB()
//{
//	if (!_bFullTest)
//	{
//		QSKIP("Skipping....", SkipSingle);
//	}
//	
//	BoardPtr board(new Board("http://www.anothermessageboard.com"));
//	board->setParser(ParserBasePtr(new VBulletin3X(board->getUrl())));
//
//	LoginInfo		loginInfo("TestUser", "testpassword");
//	StringMap	results = board->getParser()->login(loginInfo);
//
//	QCOMPARE(results.getBool("success"), true);
//	QCOMPARE(results.has("securityToken"), true);
//	QCOMPARE(results.has("sessionKey"), true);
//
//	board->crawlRoot(false);
//
//	ForumPtr root = board->getRoot();
//	ForumList rootList = root->getForums();
//
//    QCOMPARE(rootList.size(), 3);   
//	
//	ForumPtr forum = rootList.at(0);
//	QCOMPARE(rootList.at(0)->getName(), QString("General Topics"));
//	QCOMPARE(rootList.at(1)->getName(), QString("AMB Discussion"));
//	QCOMPARE(rootList.at(2)->getName(), QString("AMB After Dark"));
//
//	ForumList genTopList = board->getParser()->getForumList(rootList.at(0)->getId());
//	QCOMPARE(genTopList.size(), 3);
//
//	QCOMPARE(genTopList.at(0)->getName(), QString("General"));
//	QCOMPARE(genTopList.at(2)->getName(), QString("Picture Essays"));
//
//	ForumList forum66 = board->getParser()->getForumList("66");
//	QCOMPARE(forum66.size(), 5);
//	QCOMPARE(forum66.at(3)->getForumType(), owl::Forum::LINK);
//}
//
//void VBulletin3XTests::testJUOT()
//{
//	if (!_bFullTest)
//	{
//		QSKIP("Skipping....", SkipSingle);
//	}
//	
//	BoardPtr board(new Board("http://www.juot.net/forums"));
//	board->setParser(ParserBasePtr(new VBulletin3X(board->getUrl())));
//
//	board->crawlRoot(false);
//
//	ForumList rootList = board->getRoot()->getForums();
//
//	QCOMPARE(rootList.size(), 3);
//	QCOMPARE(rootList.at(0)->getName(), QString("Joe's Ultimate Off Topic"));
//	QCOMPARE(rootList.at(1)->getName(), QString("Joe's Ultimate On Topic"));
//	QCOMPARE(rootList.at(2)->getName(), QString("Forum Support & Administration"));
//
//	ForumList tempList = board->getParser()->getForumList("12");
//	QCOMPARE(tempList.size(), 1);
//
//	ForumPtr forumPtr = tempList.at(0);
//	QCOMPARE(forumPtr->getName(), QString("Online Deals"));
//
//	//// forum 19 has 3 subs for users and 2 subs forums for guests
//	tempList.clear();
//	tempList = board->getParser()->getForumList("19");
//	QCOMPARE(tempList.size(), 2);
//
//	forumPtr = tempList.at(1);
//	QCOMPARE(forumPtr->getName(), QString("Forum Suggestions, Help, and Tech Support"));
//}
//
//void VBulletin3XTests::testGAT() 
//{
//	if (!_bFullTest)
//	{
//		QSKIP("Skipping....", SkipSingle);
//	}
//
//	ForumList		rootList;
//	BoardPtr board(new Board("http://generalanytopic.com/forum"));
//	board->setParser(ParserBasePtr(new VBulletin3X(board->getUrl())));
//
//	board->crawlRoot(false);
//
//	QCOMPARE(board->getRoot()->getForums().size(), 2);
//	QCOMPARE(board->getRoot()->getForums().at(0)->getName(), QString("General Stuff"));
//	QCOMPARE(board->getRoot()->getForums().at(1)->getName(), QString("The \"Post Here Twice\" Forum"));
//}
//
//void VBulletin3XTests::testBehindTheVoiceActors()
//{
//	BoardPtr board(new Board("http://www.behindthevoiceactors.com/forums"));
//	board->setParser(ParserBasePtr(new VBulletin3X(board->getUrl())));
//	board->crawlRoot(false);
//
//	ForumPtr root = board->getRoot();
//
//	QCOMPARE(board->getRoot()->getForums().size(), 13);
//	QCOMPARE(board->getRoot()->getForums().at(0)->getName(), QString("The Art of Voice Acting"));
//	QCOMPARE(board->getRoot()->getForums().at(1)->getName(), QString("Voice Actors"));
//	QCOMPARE(board->getRoot()->getForums().at(2)->getName(), QString("Animation Central"));
//	QCOMPARE(board->getRoot()->getForums().at(3)->getName(), QString("Movie World"));
//	QCOMPARE(board->getRoot()->getForums().at(4)->getName(), QString("Gaming Central"));
//	QCOMPARE(board->getRoot()->getForums().at(5)->getName(), QString("General Topics"));
//	QCOMPARE(board->getRoot()->getForums().at(6)->getName(), QString("Voice Compare"));
//	QCOMPARE(board->getRoot()->getForums().at(7)->getName(), QString("Request Center"));
//	QCOMPARE(board->getRoot()->getForums().at(8)->getName(), QString("BTVA Site Features"));
//	QCOMPARE(board->getRoot()->getForums().at(9)->getName(), QString("Site Helpers"));
//	QCOMPARE(board->getRoot()->getForums().at(10)->getName(), QString("BTVA Awards"));
//	QCOMPARE(board->getRoot()->getForums().at(11)->getName(), QString("Top 10/100 Team Lists"));
//	QCOMPARE(board->getRoot()->getForums().at(12)->getName(), QString("Act Your Voice Out - BTVA's Voice Acting Contest"));
//}
//
//void VBulletin3XTests::testNR()
//{
//	BoardPtr board(new Board("http://www.nr.com/forum"));
//	board->setParser(ParserBasePtr(new VBulletin3X(board->getUrl())));
//	board->crawlRoot(false);
//
//	ForumPtr root = board->getRoot();
//
//	QCOMPARE(root->getForums().size(), 3);
//	QCOMPARE(root->getForums().at(0)->getName(), QString("Numerical Recipes Official Announcements"));
//	QCOMPARE(root->getForums().at(1)->getName(), QString("Numerical Recipes Third Edition Forum"));
//	QCOMPARE(root->getForums().at(2)->getName(), QString("Obsolete Editions Forum"));
//}
//
//void VBulletin3XTests::testStringForum()
//{
//	BoardPtr board(new Board("http://www.stringforum.net/board"));
//	board->setParser(ParserBasePtr(new VBulletin3X(board->getUrl())));
//	
//	board->crawlRoot(false);
//
//	ForumPtr root = board->getRoot();
//
//	QCOMPARE(root->getForums().size(), 6);
//	QCOMPARE(root->getForums().at(0)->getName(), QString("Strings/Racquets/Stringing"));
//	QCOMPARE(root->getForums().at(1)->getName(), QString("Localized Forums"));
//	QCOMPARE(root->getForums().at(2)->getName(), QString("Stringforum.net Partners"));
//	QCOMPARE(root->getForums().at(3)->getName(), QString("Classifieds"));
//	QCOMPARE(root->getForums().at(4)->getName(), QString("Stringforum Comments"));
//	QCOMPARE(root->getForums().at(5)->getName(), QString("Miscellaneous"));
//}
//
//void VBulletin3XTests::testDetroitLionsForum()
//{
//	BoardPtr board(new Board("http://detroitlionsforum.com/forums"));
//	board->setParser(ParserBasePtr(new VBulletin3X(board->getUrl())));
//	board->crawlRoot(false);
//
//	ForumPtr root = board->getRoot();
//
//	QCOMPARE(root->getForums().size(), 2);
//	QCOMPARE(root->getForums().at(0)->getName(), QString("Sports Discussion Forums"));
//	QCOMPARE(root->getForums().at(1)->getName(), QString("General Discussion Forums"));
//}
//
//void VBulletin3XTests::testMacRumors()
//{
//	BoardPtr board(new Board("http://forums.macrumors.com"));
//	board->setParser(ParserBasePtr(new VBulletin3X(board->getUrl())));
//	board->crawlRoot(false);
//
//	ForumPtr root = board->getRoot();
//
//	QCOMPARE(root->getForums().size(), 9);
//	QCOMPARE(root->getForums().at(0)->getName(), QString("News and Article Discussion"));
//	QCOMPARE(root->getForums().at(1)->getName(), QString("iPhone, iPod and iPad"));
//	QCOMPARE(root->getForums().at(2)->getName(), QString("Apple Applications"));
//	QCOMPARE(root->getForums().at(3)->getName(), QString("Apple Hardware"));
//	QCOMPARE(root->getForums().at(4)->getName(), QString("Apple Systems and Services"));
//	QCOMPARE(root->getForums().at(5)->getName(), QString("Special Interests"));
//	QCOMPARE(root->getForums().at(6)->getName(), QString("Mac Community"));
//	QCOMPARE(root->getForums().at(7)->getName(), QString("Private Forums"));
//	QCOMPARE(root->getForums().at(8)->getName(), QString("Archive"));
//}
//
//void VBulletin3XTests::testJeepsUnlimited()
//{
//	BoardPtr board(new Board("http://www.jeepsunlimited.com/forums"));
//	board->setParser(ParserBasePtr(new VBulletin3X(board->getUrl())));
//	board->crawlRoot(false);
//
//	ForumPtr root = board->getRoot();
//
//	QCOMPARE(root->getForums().size(), 6);
//	QCOMPARE(root->getForums().at(0)->getName(), QString("Platform Specific Tech Forums"));
//	QCOMPARE(root->getForums().at(1)->getName(), QString("Marketplace"));
//	QCOMPARE(root->getForums().at(2)->getName(), QString("Special Automotive Interests"));
//	QCOMPARE(root->getForums().at(3)->getName(), QString("Regional Forums"));
//	QCOMPARE(root->getForums().at(4)->getName(), QString("etc"));
//	QCOMPARE(root->getForums().at(5)->getName(), QString("Forums Support & Administration"));
//}
//
//void VBulletin3XTests::testGirlSoutWest()
//{
//	BoardPtr board(new Board("http://girlsoutwest.com/vbulletin"));
//	board->setParser(ParserBasePtr(new VBulletin3X(board->getUrl())));
//	board->crawlRoot(false);
//
//	ForumPtr root = board->getRoot();
//
//	QCOMPARE(root->getForums().size(), 1);
//	QCOMPARE(root->getForums().at(0)->getName(), QString("Girlsoutwest Website"));
//}
//
//void VBulletin3XTests::testArkansasCrawlers()
//{
//	BoardPtr board(new Board("http://www.arkansascrawlers.com/main/forum"));
//	board->setParser(ParserBasePtr(new VBulletin3X(board->getUrl())));
//	board->crawlRoot(false);
//
//	ForumPtr root = board->getRoot();
//
//	QCOMPARE(root->getForums().size(), 5);
//	QCOMPARE(root->getForums().at(0)->getName(), QString("Arkansas Crawler Sponsors Club Deals & Promotions"));
//	QCOMPARE(root->getForums().at(1)->getName(), QString("Public"));
//	QCOMPARE(root->getForums().at(2)->getName(), QString("Events-Trail Rides"));
//	QCOMPARE(root->getForums().at(3)->getName(), QString("For Sale / WTB"));
//	QCOMPARE(root->getForums().at(4)->getName(), QString("Help"));
//}
//
//void VBulletin3XTests::testPCGamer()
//{
//	BoardPtr board(new Board("http://www.pcgamer.com/forum"));
//	board->setParser(ParserBasePtr(new VBulletin3X(board->getUrl())));
//	board->crawlRoot(false);
//
//	ForumPtr root = board->getRoot();
//
//	QCOMPARE(root->getForums().size(), 5);
//	QCOMPARE(root->getForums().at(0)->getName(), QString("Magazine and site talk"));
//	QCOMPARE(root->getForums().at(1)->getName(), QString("Games"));
//	QCOMPARE(root->getForums().at(2)->getName(), QString("Play with PC Gamer"));
//	QCOMPARE(root->getForums().at(3)->getName(), QString("Tech"));
//	QCOMPARE(root->getForums().at(4)->getName(), QString("Off topic"));
//}
//
//void VBulletin3XTests::testSEQAnswers()
//{
//	BoardPtr board(new Board("http://seqanswers.com/forums"));
//	board->setParser(ParserBasePtr(new VBulletin3X(board->getUrl())));
//	board->crawlRoot(false);
//
//	ForumPtr root = board->getRoot();
//
//	QCOMPARE(root->getForums().size(), 12);
//	QCOMPARE(root->getForums().at(0)->getName(), QString("Introductions"));
//	QCOMPARE(root->getForums().at(1)->getName(), QString("General"));
//	QCOMPARE(root->getForums().at(2)->getName(), QString("Core Facilities"));
//	QCOMPARE(root->getForums().at(3)->getName(), QString("Literature Watch"));
//	QCOMPARE(root->getForums().at(4)->getName(), QString("Events / Conferences"));
//	QCOMPARE(root->getForums().at(5)->getName(), QString("Bioinformatics"));
//	QCOMPARE(root->getForums().at(6)->getName(), QString("Jobs Forums"));
//	QCOMPARE(root->getForums().at(7)->getName(), QString("Sequencing Technologies/Companies"));
//	QCOMPARE(root->getForums().at(8)->getName(), QString("Applications Forums"));
//	QCOMPARE(root->getForums().at(9)->getName(), QString("Regional/Local Communities"));
//	QCOMPARE(root->getForums().at(10)->getName(), QString("Personalized Genomics"));
//	QCOMPARE(root->getForums().at(11)->getName(), QString("Site News"));
//}

//#include "ParserTests.moc"
