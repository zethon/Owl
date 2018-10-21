// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <QtTest/QtTest>

class OwlUtilsTests: public QObject
{
	Q_OBJECT

private Q_SLOTS:
    void sanitizeUrlTest();
    void versionTest();
    void StringMapTest();   
    void MomentTest();
    void PreviewTextTest();
};
