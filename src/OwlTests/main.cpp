#include <iostream>
#include <QtTest/QtTest>
#include <log4qt/consoleappender.h>
#include <log4qt/logger.h>
#include <log4qt/logmanager.h>
#include <log4qt/ttcclayout.h>
#include "OwlUtilsTests.h"
#include "SampleTests.h"
#include "ParserTests.h"
#include "BBCodeParserTests.h"

void initConsoleAppender()
{
	using Log4Qt::TTCCLayout;
	using Log4Qt::ConsoleAppender;

	TTCCLayout *p_layout = new TTCCLayout();
	p_layout->setDateFormat(Log4Qt::TTCCLayout::DateFormat::ABSOLUTEFMT);
	p_layout->activateOptions();

	ConsoleAppender *p_appender = new ConsoleAppender(p_layout, ConsoleAppender::STDOUT_TARGET);
	p_appender->activateOptions();

	// Set appender on root logger
	Log4Qt::Logger::rootLogger()->addAppender(p_appender);

#ifdef _DEBUG
	Log4Qt::Logger::rootLogger()->setLevel(Log4Qt::Level::TRACE_INT);
#else
	Log4Qt::Logger::rootLogger()->setLevel(Log4Qt::Level::INFO_INT);
#endif
}

int main(int argc, char *argv[])
{
	auto iRetVal = 0;

    QApplication app(argc, argv);
	initConsoleAppender();
    
//#ifdef Q_OS_WIN
//	QString parserFolder("C:/vbulletin/deltas/client/parsers");
//#elif defined Q_OS_MACX
//    QString parserFolder("/Users/addy/src/deltas/client/parsers");
//#endif
    
    OwlUtilsTests utilsTests;
    BBRegExParserTests BBparserTests;
//	VBulletin3XTests vbulletin3xTests(parserFolder);

    iRetVal += QTest::qExec(&utilsTests, argc, argv);
    iRetVal += QTest::qExec(&BBparserTests, argc, argv);
//	iRetVal += QTest::qExec(&vbulletin3xTests, argc, argv);


#ifdef Q_OS_WIN
    std::cout << "Press Enter to continue...\n";
    std::string trash;
    std::getline(std::cin, trash);
#endif

	return iRetVal;
}
