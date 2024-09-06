#ifdef _WIN32

#include <Windows.h>
#include <gtest/gtest.h>
#include <zeus/foundation/system/win/wmi.h>
#include <zeus/foundation/system/os.h>
#include <zeus/foundation/system/win/window.h>

using namespace zeus;

TEST(System, OsKernelName)
{
    EXPECT_TRUE(OS::OsKernelName().find("NT") != std::string::npos);
}

TEST(System, OsKernelVersion)
{
    EXPECT_EQ(OS::OsVersion(), OS::OsKernelVersion());
}

TEST(System, OsDisplayName)
{
    EXPECT_TRUE(OS::OsDisplayName().find("Windows") != std::string::npos);
}

TEST(System, OsVersion)
{
    auto query = WinWMI::Create();
    ASSERT_TRUE(query.has_value());
    auto version = query->Query("SELECT Version FROM Win32_OperatingSystem").value();
    EXPECT_TRUE(OS::OsVersion().ToString().find(version[0]["Version"].get<std::string>()) != std::string::npos);
}

TEST(System, DetectServer)
{
    auto query = WinWMI::Create();
    ASSERT_TRUE(query.has_value());
    auto version = query->Query("SELECT Name FROM Win32_OperatingSystem").value();
    EXPECT_EQ(version[0]["Name"].get<std::string>().find("Server") != std::string::npos, zeus::OS::IsWinServer());
}

TEST(System, OsArchitecture)
{
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    const char* p = NULL;
    switch (si.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_INTEL:
        p = "X86";
        break;
    case PROCESSOR_ARCHITECTURE_MIPS:
        p = "MIPS";
        break;
    case PROCESSOR_ARCHITECTURE_ALPHA:
        p = "ALPHA";
        break;
    case PROCESSOR_ARCHITECTURE_PPC:
        p = "PPC";
        break;
    case PROCESSOR_ARCHITECTURE_ARM:
        p = "ARM";
        break;
    case PROCESSOR_ARCHITECTURE_IA64:
        p = "IA64";
        break;
#ifdef PROCESSOR_ARCHITECTURE_IA32_ON_WIN64
    case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:
        p = "IA64/32";
        break;
#endif
#ifdef PROCESSOR_ARCHITECTURE_AMD64
    case PROCESSOR_ARCHITECTURE_AMD64:
        p = "X64";
        break;
#endif
    default:
        p = "Unknown";
        break;
    }
    EXPECT_EQ(true, OS::OsArchitecture() == std::string(p));
}

TEST(System, IsWindows)
{
    EXPECT_NE(OS::OsDisplayName(), "Unknown");
    EXPECT_EQ(true, OS::IsWindows());
}

TEST(System, IsUnix)
{
    EXPECT_EQ(false, OS::IsUnix());
}

TEST(Window, base)
{
    HWND temp = NULL;
    WinWindow::EnumWindow(
        [&temp](HWND hwnd)
        {
            temp = hwnd;
            return false;
        }
    );
    EXPECT_FALSE(WinWindow::GetWindowClass(temp).empty());
    EXPECT_GT(WinWindow::GetWindowPid(temp), 0);
    EXPECT_TRUE(WinWindow::IsExist(temp));
}

#endif