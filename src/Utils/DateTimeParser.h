// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

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

	QDate parseDate(const QString& text, bool *bOk = NULL);
	QTime parseTime(const QString& text, bool *bOk = NULL);

	void Reset();

private:
	QString _dateFormat;
	QString _timeFormat;
};

class Moment final : public QObject
{

public:
    Moment();
    Moment(const QDateTime&);
    virtual ~Moment() = default;

    const QString toString() const;
    const QString toString(const QDateTime& now) const;

    void setDateTime(const QDateTime& dt) { _dt = dt; }
    const QDateTime datetime() const { return _dt; }

private:
    int monthsTo(const QDateTime& start, const QDateTime& end) const;

    QDateTime _dt;
};

} // owl namespace
