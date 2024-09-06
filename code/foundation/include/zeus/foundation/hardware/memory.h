#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace zeus::Hardware
{
struct MemoryBankImpl;
class MemoryBank
{
public:
    enum class BankType : uint8_t
    {
        kOther   = 1,
        kUnknown = 2,
        kDRAM    = 3,
        kEDRAM   = 4,
        kVRAM    = 5,
        kSRAM    = 6,
        kRAM     = 7,
        kROM     = 8,
        kFLASH   = 9,
        kEEPROM  = 0x0A,
        kFEPROM  = 0x0B,
        kEPROM   = 0x0C,
        kCDRAM   = 0x0D,
        k3DRAM   = 0x0E,
        kSDRAM   = 0x0F,
        kSGRAM   = 0x10,
        kRDRAM   = 0x11,
        kDDR     = 0x12,
        kDDR2    = 0x13,
        kDDR2_FB = 0x14,
        kDDR3    = 0x18,
        kFBD2    = 0x19,
        kDDR4    = 0x1A,
        kLPDDR   = 0x1B,
        kLPDDR2  = 0x1C,
        kLPDDR3  = 0x1D,
        kLPDDR4  = 0x1E,
        kHBM     = 0x20,
        kHBM2    = 0x21,
        kDDR5    = 0x22,
        kLPDDR5  = 0x23,
    };
    MemoryBank();
    ~MemoryBank();
    MemoryBank(const MemoryBank& other);
    MemoryBank(MemoryBank&& other) noexcept;
    MemoryBank& operator=(const MemoryBank& other);
    MemoryBank& operator=(MemoryBank&& other) noexcept;
    std::string Name() const;
    std::string Manufacturer() const;
    std::string SerialNumber() const;
    std::string PartNumber() const;
    uint64_t    Capacity() const;   //KB
    uint32_t    ColokSpeed() const; //Mhz
    uint32_t    DataWidth() const;  //bits
    BankType    Type() const;
public:
    //因为对于内存条的名称需要查表，性能耗费较大，所以通过name参数控制是否获取内存条名称
    //linux获取bank信息必须 具有root权限
    static std::vector<MemoryBank> GetMemoryBankList();
    static std::string             TypeName(BankType type);
private:
    std::unique_ptr<MemoryBankImpl> _impl;
};

struct MemoryImpl;
class Memory
{
public:
    Memory();
    ~Memory();
    Memory(const Memory& other) noexcept;
    Memory(Memory&& other) noexcept;
    Memory&                        operator=(const Memory& other) noexcept;
    Memory&                        operator=(Memory&& other) noexcept;
    uint64_t                       TotalPageCapacity() const noexcept;         //KB
    uint64_t                       VisiblePhysicalCapacity() const noexcept;   //KB
    uint64_t                       InstalledPhysicalCapacity() const noexcept; //KB
    uint64_t                       FreePhysicalCapacity() const noexcept;      //KB
    const std::vector<MemoryBank>& BankList() const noexcept;
public:
    //linux获取bank信息必须 具有root权限
    static Memory GetMemory(bool bank = false);
private:
    std::unique_ptr<MemoryImpl> _impl;
};
} // namespace zeus::Hardware

#include "zeus/foundation/core/zeus_compatible.h"
