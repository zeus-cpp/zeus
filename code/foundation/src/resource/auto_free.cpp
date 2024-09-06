#include "zeus/foundation/resource/auto_free.h"
#include <cstdlib>
namespace zeus
{
struct AutoFreeImpl
{
    void* data;
};

AutoFree::AutoFree(void* data) : _impl(std::make_unique<AutoFreeImpl>())
{
    _impl->data = data;
}

AutoFree::~AutoFree()
{
    if (_impl && _impl->data)
    {
        free(_impl->data);
    }
}

AutoFree::AutoFree(AutoFree&& other) noexcept : _impl(std::move(other._impl))
{
}
AutoFree& AutoFree::operator=(AutoFree&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}
} // namespace zeus
