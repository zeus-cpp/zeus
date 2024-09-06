#include "zeus/foundation/resource/auto_release.h"
namespace zeus
{
struct AutoReleaseImpl
{
    std::function<void()> destructor;
};

AutoRelease::AutoRelease(const std::function<void()>& destructor) : _impl(std::make_unique<AutoReleaseImpl>())
{
    _impl->destructor = destructor;
}

AutoRelease::AutoRelease(std::function<void()>&& destructor) : _impl(std::make_unique<AutoReleaseImpl>())
{
    _impl->destructor.swap(destructor);
}

AutoRelease::~AutoRelease()
{
    if (_impl && _impl->destructor)
    {
        _impl->destructor();
    }
}

AutoRelease::AutoRelease(AutoRelease&& other) noexcept : _impl(std::move(other._impl))
{
}
AutoRelease& AutoRelease::operator=(AutoRelease&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}
} // namespace zeus
