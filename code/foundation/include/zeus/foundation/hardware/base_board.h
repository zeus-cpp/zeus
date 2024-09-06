#pragma once

#include <string>
#include <memory>

namespace zeus
{
namespace Hardware
{
struct BaseBoardImpl;
class BaseBoard
{
public:
    BaseBoard();
    ~BaseBoard();
    BaseBoard(const BaseBoard& other) noexcept;
    BaseBoard(BaseBoard&& other) noexcept;
    BaseBoard&  operator=(const BaseBoard& other) noexcept;
    BaseBoard&  operator=(BaseBoard&& other) noexcept;
    std::string Manufacturer() const noexcept;
    std::string Product() const noexcept;
    std::string SerialNumber() const noexcept;
public:
    static BaseBoard GetBaseBoard();
private:
    std::unique_ptr<BaseBoardImpl> _impl;
};
} // namespace Hardware
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
