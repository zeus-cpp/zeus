#pragma once
#include <memory>

namespace zeus
{
struct AutoFreeImpl;
class AutoFree
{
public:
    AutoFree(void* data);
    ~AutoFree();
    AutoFree(const AutoFree&)            = delete;
    AutoFree& operator=(const AutoFree&) = delete;
    AutoFree(AutoFree&& other) noexcept;
    AutoFree& operator=(AutoFree&& other) noexcept;
private:
    std::unique_ptr<AutoFreeImpl> _impl;
};
}
#include "zeus/foundation/core/zeus_compatible.h"
