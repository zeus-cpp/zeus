#pragma once
#ifdef _WIN32
#include <string>
#include <memory>
#include <optional>
#include <filesystem>
#include <system_error>
#include <zeus/expected.hpp>

namespace zeus
{
struct WinFileManifestImpl;
class WinFileManifest
{
public:
    enum class Invoke
    {
        kAsInvoker            = 1,
        kHighestAvailable     = 2,
        kRequireAdministrator = 3,
    };

    WinFileManifest(const WinFileManifest&) = delete;
    WinFileManifest(WinFileManifest&& other) noexcept;
    WinFileManifest& operator=(const WinFileManifest&) = delete;
    WinFileManifest& operator=(WinFileManifest&& other) noexcept;
    ~WinFileManifest();

    std::optional<WinFileManifest::Invoke>                  GetInvoke() const;
    std::optional<bool>                                     IsUiAccess() const;
    static zeus::expected<WinFileManifest, std::error_code> Load(const std::filesystem::path& path);
protected:
    WinFileManifest();
private:
    std::unique_ptr<WinFileManifestImpl> _impl;
};
}

#endif // _WIN32

#include "zeus/foundation/core/zeus_compatible.h"
