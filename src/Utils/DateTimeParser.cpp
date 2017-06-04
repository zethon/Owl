// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#include <QDateTime>
#include "DateTimeParser.h"

namespace owl
{

DateTimeParser::DateTimeParser()
{
	// do nothing
}

DateTimeParser::~DateTimeParser()
{
	// do nothing
}

QDate DateTimeParser::parseDate(const QString& text, bool *bOk)
{
	QDate date;
	bool bSuccess = false;

	if (!text.isEmpty())
	{
		if (text.compare("Today", Qt::CaseInsensitive) == 0)
		{
			date = QDate::currentDate();
			bSuccess = true;
		}
		else if (text.compare("Yesterday", Qt::CaseInsensitive) == 0)
		{
			date = QDate::currentDate().addDays(-1);
			bSuccess = true;
		}
		else
		{
			if (_dateFormat.isEmpty())
			{
				QStringList formats = QStringList()
					<< "MM-dd-yyyy"
					<< "d MMMM yy";

                for (QString format : formats)
				{
					date = QDate::fromString(text, format);
					if (date.isValid())
					{
						if (date.year() < 2000)
						{
                            date = date.addYears(1000);
						}

						_dateFormat = format;
						bSuccess = true;
						break;
					}
				}
			}
			else
			{
				date = QDate::fromString(text, _dateFormat);

				if (date.isValid())
				{
					bSuccess = true;
				}
			}
		}
	}

	if (bOk != NULL)
	{
		*bOk = bSuccess;
	}

	return date;
}

QTime DateTimeParser::parseTime(const QString& text, bool *bOk)
{
	QTime time;
	bool bSuccess = false;

	if (!text.isEmpty())
	{
		if (_timeFormat.isEmpty())
		{
			QStringList formats = QStringList()
				<< "hh:mm AP"
				<< "hh:mm";

            for(QString format : formats)
			{
                time = QTime::fromString(text, format);
				if (time.isValid())
				{
					_timeFormat = format;
					bSuccess = true;
					break;
				}
			}
		}
		else
		{
			time = QTime::fromString(text, _timeFormat);

			if (time.isValid())
			{
				bSuccess = true;
			}
		}
	}

	if (bOk != NULL)
	{
		*bOk = bSuccess;
	}

	return time;
}

void DateTimeParser::Reset()
{
	_dateFormat.clear();
    _timeFormat.clear();
}

Moment::Moment()
    : Moment(QDateTime())
{
    // nothing to do
}

Moment::Moment(const QDateTime &dt)
    : _dt(dt)
{
    // nothing to do
}

const QString Moment::toString() const
{
    return toString(QDateTime::currentDateTime());
}

const QString Moment::toString(const QDateTime& now) const
{
    const auto secondsTo = _dt.secsTo(now);

    if (secondsTo < 0)
    {
        return _dt.toString("yyyy-M-d hh:mm");
    }

    if (secondsTo <= 45)
    {
        return QString("A few seconds ago");
    }
    else if (secondsTo <= 90)
    {
        return QString("A minute ago");
    }

    const auto minutesTo = (int)(secondsTo / 60);
    if (minutesTo <= 45)
    {
        return QString("%1 minutes ago").arg(minutesTo);
    }
    else if (minutesTo <= 60)
    {
        return QString("An hour ago");
    }

    const auto hoursTo = (int)(minutesTo / 60);
    if (hoursTo <= 22)
    {
        return QString("%1 hours ago").arg(hoursTo);
    }
    else if (hoursTo <= 36)
    {
        return QString("A day ago");
    }

    const auto daysTo = _dt.daysTo(now);
    if (daysTo <= 25)
    {
        return QString("%1 days ago").arg(daysTo);
    }

    return _dt.date().toString();
}

int Moment::monthsTo(const QDateTime &start, const QDateTime& end) const
{
    int monthCount = 0;

    const QDate startDate = start.date();
    QDate tempDate = end.date();

    while (startDate < tempDate)
    {
        tempDate = tempDate.addMonths(-1);
        monthCount++;
    }

    return monthCount;
}

} // owl namespace
