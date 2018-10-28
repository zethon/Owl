// Owl - www.owlclient.com
// Copyright Adalid Claure <aclaure@gmail.com>

#include <stdexcept>
#include <boost/spirit/include/qi.hpp>

#include "Version.h"

namespace owl
{

Version::Version(const std::string& version)
{
    if (version.empty()) return;

    namespace bsq = boost::spirit::qi;

    // Use `bsq::copy` per this stackoverflow answer:
    // https://stackoverflow.com/questions/53033501/boost-spirit-qi-crashes-for-memory-violation
    auto versionParser = bsq::copy(
        bsq::uint_
        >> -('.' >> bsq::uint_)
        >> -('.' >> bsq::uint_)
        >> -('.' >> bsq::uint_)
    );

    auto begin = version.begin();
    if (!bsq::parse(begin, version.end(), versionParser, _iMajor, _iMinor, _iBuild, _iRevision)
        || begin != version.end())
    {
        BOOST_THROW_EXCEPTION(std::runtime_error(std::string("Invalid version string: ") + version));
    }
}

Version::Version(std::uint8_t major)
    : Version(major, 0, 0, 0)
{
}

Version::Version(std::uint8_t major, std::uint8_t minor)
    : Version(major, minor, 0, 0)
{
}

Version::Version(std::uint8_t major, std::uint8_t minor, std::uint8_t build)
    : Version(major, minor, build, 0)
{
}

Version::Version(std::uint8_t major, std::uint8_t minor, std::uint8_t build, std::uint8_t revision)
    : _iMajor(major), _iMinor(minor), _iBuild(build), _iRevision(revision)
{
}

} // owl namespace
