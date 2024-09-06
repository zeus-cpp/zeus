#ifdef __linux__
#include <thread>
#include <sys/utsname.h>
#include <gtest/gtest.h>
#include <zeus/foundation/system/os.h>

using namespace zeus;

TEST(System, OsKernelName)
{
    struct utsname uts;
    uname(&uts);
    EXPECT_EQ(uts.sysname, OS::OsKernelName());
}

TEST(System, OsKernelVersion)
{
    struct utsname uts;
    uname(&uts);
    EXPECT_EQ(uts.release, OS::OsKernelVersionString());
}

TEST(System, OsDisplayName)
{
    auto name = OS::OsDisplayName();
    EXPECT_TRUE(
        name.find("Ubuntu") != std::string::npos || name.find("CentOS") != std::string::npos || name.find("redhat") != std::string::npos ||
        name.find("Kylin") != std::string::npos || name.find("UnionTech OS") != std::string::npos
    );
}

TEST(System, OsVersion)
{
    EXPECT_FALSE(OS::OsVersion().Zero());
}

TEST(System, OsArchitecture)
{
    struct utsname uts;
    uname(&uts);

    EXPECT_EQ(uts.machine, OS::OsArchitecture());
}

TEST(System, IsUnix)
{
    EXPECT_TRUE(OS::IsUnix());
}

TEST(System, IsWindow)
{
    EXPECT_FALSE(OS::IsWindows());
}

#endif