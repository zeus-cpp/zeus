#pragma once

#include <functional>
#include <memory>
namespace zeus
{
struct AutoReleaseImpl;
class AutoRelease
{
public:
    AutoRelease(const std::function<void()>& destructor);
    AutoRelease(std::function<void()>&& destructor);
    ~AutoRelease();

    AutoRelease(const AutoRelease&)            = delete;
    AutoRelease& operator=(const AutoRelease&) = delete;
    AutoRelease(AutoRelease&& other) noexcept;
    AutoRelease& operator=(AutoRelease&& other) noexcept;

private:
    std::unique_ptr<AutoReleaseImpl> _impl;
};

#ifdef _WIN32
#define __ZEUS_CONCAT(a, b)     a##b
#define __ZEUS_MAKE_DEFER(line) zeus::AutoRelease __ZEUS_CONCAT(Defer, line) = [&]()
#define ZEUS_DEFER              __ZEUS_MAKE_DEFER(__LINE__)
#endif
/*
Usage:
    void function()
    {
        FILE *file = fopen("example.txt", "r");
        ZEUS_DEFER
        {
            fclose(file);
        };

        ...
    }
*/
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
