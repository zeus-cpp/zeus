#pragma once
#include <memory>
#include <functional>
namespace zeus
{
struct AutoConstructorImpl;
class AutoConstructor
{
public:
    AutoConstructor(const std::function<void()>& constructor) noexcept;
    AutoConstructor(std::function<void()>&& constructor) noexcept;
    ~AutoConstructor() noexcept;

    AutoConstructor(const AutoConstructor&)            = delete;
    AutoConstructor& operator=(const AutoConstructor&) = delete;
    AutoConstructor(AutoConstructor&&)                 = delete;
    AutoConstructor& operator=(AutoConstructor&&)      = delete;
private:
    std::unique_ptr<AutoConstructorImpl> _impl;
};
}

#include "zeus/foundation/core/zeus_compatible.h"
