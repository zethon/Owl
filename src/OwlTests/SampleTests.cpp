#include <QtTest/QtTest>
#include "SampleTests.h"

void SampleTests::toUpper()
{
	QString str = "Hello";
	QCOMPARE(str.toUpper(), QString("HELLO"));
}

//#include "SampleTests.moc"