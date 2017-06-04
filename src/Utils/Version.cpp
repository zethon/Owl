// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#include "Exception.h"
#include "Version.h"

namespace owl
{

Version::Version()
		: _iMajor(0), _iMinor(0), _iBuild(0), _iRevision(0)
{
	// do nothing
}

Version::Version(const char* version)
	: _iMajor(0), _iMinor(0), _iBuild(0), _iRevision(0)
{
	int			iIdx = 0;
	QString		verStr(version);

	verStr = verStr.simplified();

    for (QString part : verStr.split("."))
	{
		bool bIsOk = false;
		int iTemp = part.toInt(&bIsOk);

		if (!bIsOk)
		{
            OWL_THROW_EXCEPTION(OwlException(QString("Invalid component at index %1 parsing '%2'").arg(iIdx).arg(verStr)));
		}

		if (iIdx == 0)
		{
			_iMajor = iTemp;
		}
		else if (iIdx == 1)
		{
			_iMinor = iTemp;
		}
		else if (iIdx == 2)
		{
			_iBuild = iTemp;
		}
		else if (iIdx == 3)
		{
			_iRevision = iTemp;
		}
		else
		{
            OWL_THROW_EXCEPTION(OwlException(QString("Invalid version string, too many components. '%1'").arg(verStr)));
		}

		iIdx++;
	}
}

Version::Version(int major, int minor)
	: _iMajor(major), _iMinor(minor), _iBuild(0), _iRevision(0)
{

}

Version::Version(int major, int minor, int build)
	: _iMajor(major), _iMinor(minor), _iBuild(build), _iRevision(0)
{

}

Version::Version(int major, int minor, int build, int revision)
	: _iMajor(major), _iMinor(minor), _iBuild(build), _iRevision(revision)
{

}

Version::~Version()
{

}
	
} // owl namespace
