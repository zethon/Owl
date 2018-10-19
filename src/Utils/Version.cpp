// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#include "Exception.h"
#include "Version.h"
#include <boost/spirit/include/qi.hpp>

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
    namespace bsq = boost::spirit::qi;

    auto versionParser =
         bsq::uint_ 
        >> -('.' >> bsq::uint_)
        >> -('.' >> bsq::uint_)
        >> -('.' >> bsq::uint_);

    std::string temp(version);
    if (!bsq::parse(temp.begin(), temp.end(), versionParser, _iMajor, _iMinor, _iBuild, _iRevision))
    {
        OWL_THROW_EXCEPTION(OwlException(QString("Invalid version string '%1'").arg(version)));
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
