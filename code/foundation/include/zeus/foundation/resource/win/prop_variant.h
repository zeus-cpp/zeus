#pragma once
#ifdef _WIN32
#include <memory>
#include <combaseapi.h>

namespace zeus
{
struct WinPropVariantImpl;
class WinPropVariant
{
public:
    WinPropVariant();
    WinPropVariant(const WinPropVariant& other) noexcept;
    WinPropVariant(WinPropVariant&& other) noexcept;
    WinPropVariant& operator=(const WinPropVariant&) = delete;
    WinPropVariant& operator=(WinPropVariant&&)      = delete;
    ~WinPropVariant();
    operator PROPVARIANT&() noexcept;
    operator const PROPVARIANT&() const noexcept;
    PROPVARIANT* operator&() noexcept;
    PROPVARIANT* operator->() const noexcept;
    operator bool() const noexcept;
    PROPVARIANT&       Data() noexcept;
    const PROPVARIANT& Data() const noexcept;
    bool               Empty() const noexcept;
private:
    std::unique_ptr<WinPropVariantImpl> _impl;
};
} // namespace zeus
#endif
#include "zeus/foundation/core/zeus_compatible.h"
