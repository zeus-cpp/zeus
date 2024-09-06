#ifdef _WIN32

#include <gtest/gtest.h>

#include <Windows.h>
#include <powerbase.h>
#pragma comment(lib, "PowrProf.lib")

#include <zeus/foundation/system/win/power_request.h>

using namespace zeus;

bool HasDisplayPowerRequest()
{
    ULONG state = 0;
    ::CallNtPowerInformation(SystemExecutionState, nullptr, 0, &state, sizeof(state));
    return (state & ES_DISPLAY_REQUIRED) ? true : false;
}

bool HasSystemPowerRequest()
{
    ULONG state = 0;
    ::CallNtPowerInformation(SystemExecutionState, nullptr, 0, &state, sizeof(state));
    return (state & ES_SYSTEM_REQUIRED) ? true : false;
}

TEST(WinPowerRequest, Display)
{
    auto hasPowerRequest = []() -> auto
    {
        return HasDisplayPowerRequest();
    };

    // firstly, no power request presents
    ASSERT_FALSE(hasPowerRequest());

    {
        constexpr auto type   = WinPowerRequestType::DisplayRequried;
        const auto     reason = std::string("PowerRequest Test");

        const auto powerRequest = WinPowerRequestFactory::Create(type, reason);
        ASSERT_TRUE(powerRequest.has_value());

        // now, we request the power state
        ASSERT_TRUE(hasPowerRequest());
        ASSERT_TRUE(powerRequest.value()->Type() == type);
        ASSERT_TRUE(powerRequest.value()->Reason() == reason);
    }

    // finally, no power request presents
    ASSERT_FALSE(hasPowerRequest());
}

TEST(WinPowerRequest, System)
{
    auto hasPowerRequest = []() -> auto
    {
        return HasSystemPowerRequest();
    };

    // firstly, no power request presents
    ASSERT_FALSE(hasPowerRequest());

    {
        constexpr auto type   = WinPowerRequestType::SystemRequired;
        const auto     reason = std::string("PowerRequest Test");

        const auto powerRequest = WinPowerRequestFactory::Create(type, reason);
        ASSERT_TRUE(powerRequest.has_value());

        // now, we request the power state
        ASSERT_TRUE(hasPowerRequest());
        ASSERT_TRUE(powerRequest.value()->Type() == type);
        ASSERT_TRUE(powerRequest.value()->Reason() == reason);
    }

    // finally, no power request presents
    ASSERT_FALSE(hasPowerRequest());
}

#endif
