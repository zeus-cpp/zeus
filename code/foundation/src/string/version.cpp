#include "zeus/foundation/string/version.h"
#include <sstream>
#include <algorithm>
#include <regex>
namespace zeus
{

struct VersionImpl
{
    size_t _major = 0;
    size_t _minor = 0;
    size_t _patch = 0;
    size_t _build = 0;
};
Version::Version() : _impl(std::make_unique<VersionImpl>())
{
}
Version::Version(size_t major, size_t minor, size_t patch, size_t build) : _impl(std::make_unique<VersionImpl>())

{
    _impl->_major = major;
    _impl->_minor = minor;
    _impl->_patch = patch;
    _impl->_build = build;
}

Version::Version(const Version& other) : _impl(std::make_unique<VersionImpl>(*other._impl))
{
}

Version::Version(Version&& other) noexcept : _impl(std::move(other._impl))
{
}

Version::~Version()
{
}

Version& Version::operator=(const Version& other)
{
    if (this != &other)
    {
        *_impl = *other._impl;
    }
    return *this;
}

Version& Version::operator=(Version&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}

bool Version::operator==(const std::string& other) const
{
    auto otherVersion = Parse(other);
    if (otherVersion)
    {
        return *this == otherVersion;
    }
    else
    {
        return false;
    }
}

bool Version::operator==(const Version& other) const
{
    return _impl->_major == other._impl->_major && _impl->_minor == other._impl->_minor && _impl->_patch == other._impl->_patch &&
           _impl->_build == other._impl->_build;
}
bool Version::operator>(const Version& other) const
{
    if (_impl->_major > other._impl->_major)
    {
        return true;
    }
    else if (_impl->_major < other._impl->_major)
    {
        return false;
    }

    if (_impl->_minor > other._impl->_minor)
    {
        return true;
    }
    else if (_impl->_minor < other._impl->_minor)
    {
        return false;
    }

    if (_impl->_patch > other._impl->_patch)
    {
        return true;
    }
    else if (_impl->_patch < other._impl->_patch)
    {
        return false;
    }

    if (_impl->_build > other._impl->_build)
    {
        return true;
    }
    else if (_impl->_build < other._impl->_build)
    {
        return false;
    }
    return false;
}

bool Version::operator>=(const Version& other) const
{
    return this->operator==(other) || this->operator>(other);
}
bool Version::operator<(const Version& other) const
{
    return !this->operator>=(other);
}
bool Version::operator<=(const Version& other) const
{
    return !this->operator>(other);
}

Version::operator std::string() const
{
    return ToString();
}

size_t Version::GetMajor() const
{
    return _impl->_major;
}
size_t Version::GetMinor() const
{
    return _impl->_minor;
}
size_t Version::GetPatch() const
{
    return _impl->_patch;
}
size_t Version::GetBuild() const
{
    return _impl->_build;
}
void Version::SetMajor(size_t data)
{
    _impl->_major = data;
}
void Version::SetMinor(size_t data)
{
    _impl->_minor = data;
}
void Version::SetPatch(size_t data)
{
    _impl->_patch = data;
}
void Version::SetBuild(size_t data)
{
    _impl->_build = data;
}
bool Version::Zero() const
{
    return 0 == _impl->_major && 0 == _impl->_minor && 0 == _impl->_patch && 0 == _impl->_build;
}
std::string Version::ToString() const
{
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    oss << _impl->_major << "." << _impl->_minor << "." << _impl->_patch << "." << _impl->_build;
    return oss.str();
}

std::optional<Version> Version::Parse(const std::string& versiongString, bool strict)
{
    try
    {
        std::regex  reg(R"(([0-9]+)([\.-][0-9]+)?([\.-][0-9]+)?([\.-][0-9]+)?)");
        std::smatch match;
        if ((strict ? std::regex_match(versiongString, match, reg) : std::regex_search(versiongString, match, reg)) && match.size() >= 5)
        {
            auto ver           = std::optional<Version>(std::in_place, 0, 0, 0, 0);
            ver->_impl->_major = std::stoul(match[1].str());
            if (match[2].matched)
            {
                ver->_impl->_minor = std::stoul(match[2].str().substr(1));
            }
            if (match[3].matched)
            {
                ver->_impl->_patch = std::stoul(match[3].str().substr(1));
            }
            if (match[4].matched)
            {
                ver->_impl->_build = std::stoul(match[4].str().substr(1));
            }
            return ver;
        }
        else
        {
            return std::nullopt;
        }
    }
    catch (std::exception&)
    {
        return std::nullopt;
    }
}
} // namespace zeus
