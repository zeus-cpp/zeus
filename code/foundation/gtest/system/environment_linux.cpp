#ifdef __linux__

#include <gtest/gtest.h>
#include <zeus/foundation/system/environment.h>
#include <zeus/foundation/system/environment_variable.h>

using namespace zeus;

TEST(Environment, GetHome)
{
    EXPECT_EQ(EnvironmentVariable::Get("HOME"), Environment::GetHome().u8string());
}

TEST(Environment, GetDesktop)
{
    Environment::GetDesktopPathString();
    // TODO test against xdg-user-dir DESKTOP
}

#endif
