// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <QtCore>

namespace owl
{

class Version
{
public:
	enum Format
	{
		MINMAJBUILD,
		MINMAJBUILDREV
	};

	Version();
	Version(const char* version);
	Version(int major, int minor);
	Version(int major, int minor, int build);
	Version(int major, int minor, int build, int revision);

	virtual ~Version();

	int getMajor() const { return _iMajor; }
	int getMinor() const { return _iMinor; }
	int getBuild() const { return _iBuild; }
	int getRevision() const { return _iRevision; }

	bool operator==(const Version& other)
	{
		return (_iMajor == other.getMajor())
			&& (_iMinor == other.getMinor())
			&& (_iBuild == other.getBuild())
			&& (_iRevision == other.getRevision());
	}

	bool operator!=(const Version& other)
	{
		return !(*this == other);
	}

	bool operator>(const Version& other)
	{
		bool bRet = false;

		if (_iMajor > other.getMajor())
		{
			bRet = true;
		}
		else if (_iMajor == other.getMajor())
		{
			if (_iMinor > other.getMinor())
			{
				bRet = true;
			}
			else if (_iMinor == other.getMinor())
			{
				if (_iBuild > other.getBuild())
				{
					bRet = true;
				}
				else if (_iBuild == other.getBuild())
				{
					if (_iRevision > other.getRevision())
					{
						bRet = true;
					}
				}
			}
		}

		return bRet;
	}

	bool operator<(const Version& other)
	{
		bool bRet = false;
		
		if (*this != other)
		{
			bRet = !(*this > other);
		}

		return bRet;
	}

	QString toString(Version::Format fmt = MINMAJBUILD)
	{
		QString retStr;
		QTextStream stream(&retStr);
		stream << _iMajor << "." << _iMinor << "." << _iBuild;

		if (fmt == Version::MINMAJBUILDREV)
		{
			stream << _iRevision;
		}

		return retStr;
	}

private:
	int _iMajor, _iMinor, _iBuild, _iRevision;
};

} // owl namespace
