#pragma once
#include <functional>
#include <memory>
namespace zeus
{
struct AutoRaiiImpl;
class AutoRaii
{
public:
    AutoRaii(const std::function<void()>& constructor, const std::function<void()>& destructor);
    AutoRaii(std::function<void()>&& constructor, std::function<void()>&& destructor);
    ~AutoRaii();

    AutoRaii(const AutoRaii&)            = delete;
    AutoRaii& operator=(const AutoRaii&) = delete;
    AutoRaii(AutoRaii&& other) noexcept;
    AutoRaii& operator=(AutoRaii&& other) noexcept;

private:
    std::unique_ptr<AutoRaiiImpl> _impl;
};
}

#include "zeus/foundation/core/zeus_compatible.h"
