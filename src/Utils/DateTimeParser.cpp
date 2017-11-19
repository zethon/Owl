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

} // owl namespace
