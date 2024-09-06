#ifdef _WIN32
#include <Windows.h>
#include <gtest/gtest.h>
#include <zeus/foundation/resource/win/handle.h>

using namespace zeus;
TEST(WinHandle, base)
{
    HANDLE event = CreateEvent(nullptr, true, FALSE, nullptr);
    DWORD  flag  = 0;
    EXPECT_TRUE(GetHandleInformation(event, &flag));
    WinHandle handle(event);
    EXPECT_TRUE(!!handle);
    EXPECT_FALSE(handle.Empty());
    handle.Detach();
    EXPECT_FALSE(!!handle);
    EXPECT_TRUE(handle.Empty());
    EXPECT_TRUE(GetHandleInformation(event, &flag));
    handle.Attach(event);
    EXPECT_TRUE(!!handle);
    EXPECT_FALSE(handle.Empty());
    EXPECT_TRUE(GetHandleInformation(event, &flag));
    handle.Close();
    EXPECT_FALSE(!!handle);
    EXPECT_TRUE(handle.Empty());
    EXPECT_FALSE(GetHandleInformation(event, &flag));
}

TEST(WinHandle, Inherited)
{
    {
        SECURITY_ATTRIBUTES sa  = {};
        sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = nullptr;
        sa.bInheritHandle       = FALSE;
        WinHandle handle        = CreateEventW(&sa, FALSE, FALSE, nullptr);
        ASSERT_TRUE(!!handle);
        ASSERT_FALSE(handle.Empty());
        EXPECT_FALSE(handle.IsInherited().value());
        EXPECT_TRUE(handle.SetInherite(true));
        EXPECT_TRUE(handle.IsInherited().value());
        EXPECT_TRUE(handle.SetInherite(false));
        EXPECT_FALSE(handle.IsInherited().value());
        auto dup1 = *handle.Duplicate(true);
        EXPECT_NE(dup1, handle);
        EXPECT_TRUE(dup1.IsInherited().value());
        auto dup2 = *handle.Duplicate(false);
        EXPECT_NE(dup2, handle);
        EXPECT_NE(dup2, dup1);
        EXPECT_FALSE(dup2.IsInherited().value());
    }
    {
        SECURITY_ATTRIBUTES sa  = {};
        sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = nullptr;
        sa.bInheritHandle       = TRUE;
        WinHandle handle        = CreateEventW(&sa, FALSE, FALSE, nullptr);
        ASSERT_TRUE(!!handle);
        ASSERT_FALSE(handle.Empty());
        EXPECT_TRUE(handle.IsInherited().value());
        EXPECT_TRUE(handle.SetInherite(false));
        EXPECT_FALSE(handle.IsInherited().value());
        EXPECT_TRUE(handle.SetInherite(true));
        EXPECT_TRUE(handle.IsInherited().value());
        auto dup1 = *handle.Duplicate(true);
        EXPECT_NE(dup1, handle);
        EXPECT_TRUE(dup1.IsInherited().value());
        auto dup2 = *handle.Duplicate(false);
        EXPECT_NE(dup2, handle);
        EXPECT_NE(dup2, dup1);
        EXPECT_FALSE(dup2.IsInherited().value());
    }
}
#endif