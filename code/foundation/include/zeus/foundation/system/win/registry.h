#pragma once
#ifdef _WIN32
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <system_error>
#include <cstdint>
#include "zeus/expected.hpp"
#include "zeus/foundation/core/win/win_windef.h"

namespace zeus
{
struct WinRegistryImpl;
class WinRegistry
{
public:
    enum class ViewType
    {
        AUTO,
        WIN_KEY_32,
        WIN_KEY_64,
    };

    enum class ValueType
    {
        kNONE                       = 0,
        kSTRING                     = 1,
        kSTRING_EXPAND              = 2,
        kBINARY                     = 3,
        kDWORD                      = 4,
        kDWORD_BIG_ENDIAN           = 5,
        kLINK                       = 6,
        kMULTI_STRING               = 7,
        kRESOURCE_LIST              = 8,
        kFULL_RESOURCE_DESCRIPTOR   = 9,
        kRESOURCE_REQUIREMENTS_LIST = 10,
        kQWORD                      = 11,
    };
    WinRegistry();
    WinRegistry(const WinRegistry&) = delete;
    WinRegistry(WinRegistry&& other) noexcept;
    WinRegistry& operator=(const WinRegistry&) = delete;
    WinRegistry& operator=(WinRegistry&& other) noexcept;
    ~WinRegistry();

    operator HKEY() const noexcept;
    HKEY Key() const noexcept;

    zeus::expected<ValueType, std::error_code>                GetType(const std::string& name) const;
    zeus::expected<uint32_t, std::error_code>                 GetDwordValue(const std::string& name) const;
    zeus::expected<uint64_t, std::error_code>                 GetQwordValue(const std::string& name) const;
    zeus::expected<std::string, std::error_code>              GetStringValue(const std::string& name) const;
    zeus::expected<std::string, std::error_code>              GetStringExpandValue(const std::string& name) const;
    zeus::expected<std::vector<std::string>, std::error_code> GetMultiStringValue(const std::string& name) const;
    zeus::expected<std::vector<uint8_t>, std::error_code>     GetBinaryValue(const std::string& name) const;
    zeus::expected<std::vector<uint8_t>, std::error_code>     GetDataValue(const std::string& name) const;
    zeus::expected<std::vector<std::string>, std::error_code> GetSubKeys() const;
    zeus::expected<std::vector<std::string>, std::error_code> GetValueNames() const;
    zeus::expected<bool, std::error_code>                     ExistsValue(const std::string& name) const;
    zeus::expected<void, std::error_code>                     DeleteValue(const std::string& name);
    zeus::expected<void, std::error_code>                     SetDwordValue(const std::string& name, uint32_t value);
    zeus::expected<void, std::error_code>                     SetQwordValue(const std::string& name, uint64_t value);
    zeus::expected<void, std::error_code>                     SetStringValue(const std::string& name, const std::string& value);
    zeus::expected<void, std::error_code>                     SetStringExpandValue(const std::string& name, const std::string& value);
    zeus::expected<void, std::error_code>                     SetMultiStringValue(const std::string& name, const std::vector<std::string>& value);
    zeus::expected<void, std::error_code>                     SetBinaryValue(const std::string& name, const void* data, size_t size);
    zeus::expected<void, std::error_code>                     SetDataValue(const std::string& name, ValueType type, const void* value, size_t size);
    zeus::expected<void, std::error_code>                     Close();
    zeus::expected<void, std::error_code>                     Flush();
public:
    static zeus::expected<void, std::error_code>        DeleteKey(const std::string& key);
    static zeus::expected<void, std::error_code>        DeleteKey(HKEY rootKey, const std::string& subKey);
    static zeus::expected<bool, std::error_code>        ExistsKey(const std::string& key, ViewType viewType = ViewType ::AUTO);
    static zeus::expected<bool, std::error_code>        ExistsKey(HKEY rootKey, const std::string& subKey, ViewType viewType = ViewType ::AUTO);
    static zeus::expected<WinRegistry, std::error_code> OpenKey(
        const std::string& key, bool read = true, bool write = false, ViewType viewType = ViewType ::AUTO
    );
    static zeus::expected<WinRegistry, std::error_code> OpenKey(
        HKEY rootKey, const std::string& subKey, bool read = true, bool write = false, ViewType viewType = ViewType ::AUTO
    );
    static zeus::expected<WinRegistry, std::error_code> CreateKey(
        const std::string& key, bool read = true, bool write = true, ViewType viewType = ViewType ::AUTO, bool persistence = true,
        bool backupRestore = false
    );
    static zeus::expected<WinRegistry, std::error_code> CreateKey(
        HKEY rootKey, const std::string& subKey, bool read = true, bool write = true, ViewType viewType = ViewType ::AUTO, bool persistence = true,
        bool backupRestore = false
    );
    static zeus::expected<WinRegistry, std::error_code> OpenTransactedKey(
        HANDLE transaction, const std::string& key, bool read = true, bool write = false, ViewType viewType = ViewType ::AUTO
    );
    static zeus::expected<WinRegistry, std::error_code> OpenTransactedKey(
        HANDLE transaction, HKEY rootKey, const std::string& subKey, bool read = true, bool write = false, ViewType viewType = ViewType ::AUTO
    );
    static zeus::expected<WinRegistry, std::error_code> CreateTransactedKey(
        HANDLE transaction, const std::string& key, bool read = true, bool write = true, ViewType viewType = ViewType ::AUTO, bool persistence = true,
        bool backupRestore = false
    );
    static zeus::expected<WinRegistry, std::error_code> CreateTransactedKey(
        HANDLE transaction, HKEY rootKey, const std::string& subKey, bool read = true, bool write = true, ViewType viewType = ViewType ::AUTO,
        bool persistence = true, bool backupRestore = false
    );
    static zeus::expected<WinRegistry, std::error_code> OpenCurrentUserKey(bool read = true, bool write = false);

private:
    std::unique_ptr<WinRegistryImpl> _impl;
};
} // namespace zeus
#endif

#include "zeus/foundation/core/zeus_compatible.h"
