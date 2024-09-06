#include "zeus/foundation/core/system_error.h"

namespace zeus
{

class SystemErrorCategory : public std::error_category
{
public:
    const char* name() const noexcept override;

    std::string message(int value) const override;

    std::error_condition default_error_condition(int value) const noexcept override;

    bool equivalent(const std::error_code& code, int value) const noexcept override;
};

const char* SystemErrorCategory::name() const noexcept
{
    return "system(zeus)";
}

std::string SystemErrorCategory::message(int value) const
{
    return SystemErrorMessage(value);
}

std::error_condition SystemErrorCategory::default_error_condition(int value) const noexcept
{
    return std::system_category().default_error_condition(value);
}

bool SystemErrorCategory::equivalent(const std::error_code& code, int value) const noexcept
{
    return (code.value() == value) && (code.category() == SystemCategory() || code.category() == std::system_category());
}

const std::error_category& SystemCategory() noexcept
{
    static SystemErrorCategory category;
    return category;
}

} // namespace zeus
