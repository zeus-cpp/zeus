#pragma once
#include <string>
#include <memory>
#include <optional>
namespace zeus
{
struct VersionImpl;
//Major.Minor.Patch.Build
class Version
{
public:
    Version();
    Version(size_t major, size_t minor, size_t patch, size_t build);
    Version(const Version& other);
    Version(Version&& other) noexcept;
    ~Version();
    Version& operator=(const Version& other);
    Version& operator=(Version&& other) noexcept;
    bool     operator==(const std::string& other) const;
    bool     operator==(const Version& other) const;
    bool     operator>(const Version& other) const;
    bool     operator>=(const Version& other) const;
    bool     operator<(const Version& other) const;
    bool     operator<=(const Version& other) const;
    operator std::string() const;
    size_t      GetMinor() const;
    size_t      GetPatch() const;
    size_t      GetBuild() const;
    void        SetMajor(size_t data);
    void        SetMinor(size_t data);
    void        SetPatch(size_t data);
    void        SetBuild(size_t data);
    size_t      GetMajor() const;
    bool        Zero() const;
    std::string ToString() const;
public:
    //strict模式要求版本号必须是仅含有数字和.，非strict模式下，则不要求
    static std::optional<Version> Parse(const std::string& versiongString, bool strict = true);
private:
    std::unique_ptr<VersionImpl> _impl;
};

} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
