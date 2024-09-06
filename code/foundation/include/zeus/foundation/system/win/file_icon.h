#pragma once
#ifdef _WIN32
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <system_error>
#include <zeus/expected.hpp>

namespace zeus
{

struct WinFileIconImageImpl;
class WinFileIconImage
{
public:
    enum class IconItemType
    {
        kPng,
        kBmp,
    };
    ~WinFileIconImage();
    WinFileIconImage(const WinFileIconImage&) = delete;
    WinFileIconImage(WinFileIconImage&& other) noexcept;
    WinFileIconImage&                     operator=(const WinFileIconImage&) = delete;
    WinFileIconImage&                     operator=(WinFileIconImage&& other) noexcept;
    const void*                           Data() const noexcept;
    size_t                                Size() const noexcept;
    size_t                                Width() const noexcept;
    size_t                                Height() const noexcept;
    size_t                                EntryWidth() const noexcept;
    size_t                                EntryHeight() const noexcept;
    uint8_t                               ColorCount() const noexcept;
    uint16_t                              Planes() const noexcept;
    uint16_t                              BitCount() const noexcept;
    uint16_t                              Id() const noexcept;
    IconItemType                          Type() const noexcept;
    bool                                  IsStandard() const noexcept;
    zeus::expected<void, std::error_code> ExtractImage(const std::filesystem::path& path) const;
    zeus::expected<void, std::error_code> ExtractAsIcon(const std::filesystem::path& path) const;
protected:
    WinFileIconImage();
    friend class WinFileIcon;
    friend class WinFileIconGroup;
private:
    std::shared_ptr<WinFileIconImageImpl> _impl;
};

struct WinFileIconImpl;
class WinFileIcon
{
public:
    ~WinFileIcon();
    WinFileIcon(const WinFileIcon&) = delete;
    WinFileIcon(WinFileIcon&& other) noexcept;
    WinFileIcon&                          operator=(const WinFileIcon&) = delete;
    WinFileIcon&                          operator=(WinFileIcon&& other) noexcept;
    const std::vector<WinFileIconImage>&  Images() const noexcept;
    std::string                           ResourceId() const;
    zeus::expected<void, std::error_code> ExtractIconToFile(const std::filesystem::path& path, bool excludeUnStandard) const;
protected:
    WinFileIcon();
    friend class WinFileIconGroup;
private:
    std::shared_ptr<WinFileIconImpl> _impl;
};

struct WinFileIconGroupImpl;
class WinFileIconGroup
{
public:
    ~WinFileIconGroup();
    WinFileIconGroup(const WinFileIconGroup&) = delete;
    WinFileIconGroup(WinFileIconGroup&& other) noexcept;
    WinFileIconGroup&               operator=(const WinFileIconGroup&) = delete;
    WinFileIconGroup&               operator=(WinFileIconGroup&& other) noexcept;
    const std::vector<WinFileIcon>& Icons() const noexcept;
public:
    static zeus::expected<WinFileIconGroup, std::error_code> Open(const std::filesystem::path& path);
protected:
    WinFileIconGroup();
private:
    std::unique_ptr<WinFileIconGroupImpl> _impl;
};
} // namespace zeus

#endif // _WIN32

#include "zeus/foundation/core/zeus_compatible.h"
