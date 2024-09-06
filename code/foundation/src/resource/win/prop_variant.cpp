#include "zeus/foundation/resource/win/prop_variant.h"
#ifdef _WIN32
#include <propkey.h>

namespace zeus
{
struct WinPropVariantImpl
{
    PROPVARIANT data;
};
WinPropVariant::WinPropVariant() : _impl(std::make_unique<WinPropVariantImpl>())
{
    PropVariantInit(&_impl->data);
}
WinPropVariant::WinPropVariant(const WinPropVariant& other) noexcept : _impl(std::make_unique<WinPropVariantImpl>())
{
    PropVariantCopy(&_impl->data, &other._impl->data);
}
WinPropVariant::WinPropVariant(WinPropVariant&& other) noexcept : _impl(std::move(other._impl))
{
}

WinPropVariant::~WinPropVariant()
{
    if (_impl)
    {
        PropVariantClear(&_impl->data);
    }
}

WinPropVariant::operator PROPVARIANT&() noexcept
{
    return _impl->data;
}
WinPropVariant::operator const PROPVARIANT&() const noexcept
{
    return _impl->data;
}
PROPVARIANT* WinPropVariant::operator&() noexcept
{
    return &_impl->data;
}

PROPVARIANT* WinPropVariant::operator->() const noexcept
{
    return &_impl->data;
}
WinPropVariant::operator bool() const noexcept
{
    return VT_EMPTY != _impl->data.vt;
}
PROPVARIANT& WinPropVariant::Data() noexcept
{
    return _impl->data;
}
const PROPVARIANT& WinPropVariant::Data() const noexcept
{
    return _impl->data;
}
bool WinPropVariant::Empty() const noexcept
{
    return VT_EMPTY == _impl->data.vt;
}
} // namespace zeus

#endif