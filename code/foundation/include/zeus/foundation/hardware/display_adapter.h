#pragma once
#include <vector>
#include <string>
#include <memory>
#include <cstdint>
namespace zeus::Hardware
{
struct DisplayAdapterImpl;
class DisplayAdapter
{
public:
    DisplayAdapter();
    ~DisplayAdapter();
    DisplayAdapter(const DisplayAdapter& other) noexcept;
    DisplayAdapter(DisplayAdapter&& other) noexcept;
    DisplayAdapter& operator=(const DisplayAdapter& other) noexcept;
    DisplayAdapter& operator=(DisplayAdapter&& other) noexcept;
    uint64_t        SystemMemory() const noexcept;
    uint64_t        VideoMemory() const noexcept;
    uint64_t        SharedMemory() const noexcept;
    std::string     DescribeName() const;
    uint16_t        VendorId() const noexcept;
    std::string     Vendor() const noexcept;
public:
    //在32位程序中无法获取到超过3G的显存
    static std::vector<DisplayAdapter> ListAll();
private:
    std::unique_ptr<DisplayAdapterImpl> _impl;
};
} // namespace zeus::Hardware

#include "zeus/foundation/core/zeus_compatible.h"
