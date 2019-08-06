// Owl - www.owlclient.com
// Copyright (c) 2012-2019, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <QtCore>

namespace owl
{

struct DateTimeFormatOptions
{
    bool useDefault = true;
    bool usePretty = true;
    QString dateFormat;
    QString timeFormat;
};

class DateTimeParser
{
public:
	DateTimeParser();
    virtual ~DateTimeParser();

	QDate parseDate(const QString& text, bool *bOk = nullptr);
	QTime parseTime(const QString& text, bool *bOk = nullptr);

	void Reset();

private:
	QString _dateFormat;
	QString _timeFormat;
};

} // owl namespace
