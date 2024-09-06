#include "zeus/foundation/resource/auto_constructor.h"
namespace zeus
{
struct AutoConstructorImpl
{
};
AutoConstructor::AutoConstructor(const std::function<void()>& constructor) noexcept : _impl(std::make_unique<AutoConstructorImpl>())
{
    if (constructor)
    {
        constructor();
    }
}
AutoConstructor::AutoConstructor(std::function<void()>&& constructor) noexcept : _impl(std::make_unique<AutoConstructorImpl>())
{
    if (constructor)
    {
        constructor();
    }
}
AutoConstructor::~AutoConstructor() noexcept
{
}
}
