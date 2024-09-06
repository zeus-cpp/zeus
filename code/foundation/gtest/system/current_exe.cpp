#include <gtest/gtest.h>
#include <zeus/foundation/system/current_exe.h>
#include <zeus/foundation/system/process.h>
#include <zeus/foundation/string/string_utils.h>

using namespace zeus;

namespace fs = std::filesystem;

TEST(CurrentExe, PathString)
{
    EXPECT_EQ(CurrentExe::GetAppPathString(), CurrentExe::GetAppPath().u8string());
    EXPECT_EQ(CurrentExe::GetAppDirString(), CurrentExe::GetAppDir().u8string());
    EXPECT_EQ(CurrentExe::GetModulePathString(), CurrentExe::GetModulePath().u8string());
    EXPECT_EQ(CurrentExe::GetModuleDirString(), CurrentExe::GetModuleDir().u8string());
}

TEST(CurrentExe, GetAppPath)
{
    EXPECT_TRUE(IEqual(CurrentExe::GetAppPathString(), Process::GetCurrentProcess().ExePathString()));
}

TEST(CurrentExe, GetAppDir)
{
    EXPECT_TRUE(IEqual(CurrentExe::GetAppDir().u8string(), Process::GetCurrentProcess().ExePath().parent_path().u8string()));
}
