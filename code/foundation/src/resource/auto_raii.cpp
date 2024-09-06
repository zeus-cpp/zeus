#include "zeus/foundation/resource/auto_raii.h"
namespace zeus
{

struct AutoRaiiImpl
{
    std::function<void()> destructor;
};

AutoRaii::AutoRaii(const std::function<void()>& constructor, const std::function<void()>& destructor) : _impl(std::make_unique<AutoRaiiImpl>())
{
    _impl->destructor = destructor;
    if (constructor)
    {
        constructor();
    }
}
AutoRaii::AutoRaii(std::function<void()>&& constructor, std::function<void()>&& destructor) : _impl(std::make_unique<AutoRaiiImpl>())
{
    _impl->destructor.swap(destructor);
    if (constructor)
    {
        constructor();
    }
}
AutoRaii::~AutoRaii()
{
    if (_impl && _impl->destructor)
    {
        _impl->destructor();
    }
}
AutoRaii::AutoRaii(AutoRaii&& other) noexcept : _impl(std::move(other._impl))
{
}
AutoRaii& AutoRaii::operator=(AutoRaii&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}
} // namespace zeus
