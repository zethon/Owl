// Owl - www.owlclient.com
// Copyright Adalid Claure <aclaure@gmail.com>

#pragma once

#include <sstream>

namespace owl
{

class Version
{
public:

    Version() = default;
    Version(const std::string& version);

    Version(std::uint8_t major);
    Version(std::uint8_t major, std::uint8_t minor);
    Version(std::uint8_t major, std::uint8_t minor, std::uint8_t build);
    Version(std::uint8_t major, std::uint8_t minor, std::uint8_t build, std::uint8_t revision);

    virtual ~Version() = default;

    std::uint8_t getMajor() const noexcept { return _iMajor; }
    std::uint8_t getMinor() const noexcept { return _iMinor; }
    std::uint8_t getBuild() const noexcept { return _iBuild; }
    std::uint8_t getRevision() const noexcept { return _iRevision; }

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

    std::string toString(bool truncate = true) const
    {
        std::stringstream output;

        output
            << std::to_string(_iMajor)
            << "."
            << std::to_string(_iMinor);

        if (!truncate || _iBuild != 0 || _iRevision != 0)
        {
            output << "." << std::to_string(_iBuild);

            if (!truncate || _iRevision != 0)
            {
                output << "." << std::to_string(_iRevision);
            }
        }

        return output.str();
    }

private:
    std::uint8_t _iMajor = 0;
    std::uint8_t _iMinor = 0;
    std::uint8_t _iBuild = 0;
    std::uint8_t _iRevision = 0;
};

} // owl namespace
