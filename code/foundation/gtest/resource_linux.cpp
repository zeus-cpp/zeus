#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <zeus/foundation/resource/linux/file_descriptor.h>

using namespace zeus;
TEST(LinuxFileDescriptor, base)
{
    auto file = LinuxFileDescriptor(open("/dev/null", 0, O_RDWR));
    EXPECT_FALSE(file.Empty());
    file.Close();
    EXPECT_TRUE(file.Empty());
}

TEST(LinuxFileDescriptor, CloseOnExec)
{
    auto file      = LinuxFileDescriptor(open("/dev/null", O_CLOEXEC, O_RDWR));
    auto duplicate = std::move(file.Duplicate(false).value());
    EXPECT_NE(file, duplicate);
    EXPECT_FALSE(duplicate.IsCloseOnExec().value());
    EXPECT_TRUE(file.IsCloseOnExec().value());
    EXPECT_TRUE(file.SetCloseOnExec(false).has_value());
    EXPECT_FALSE(file.IsCloseOnExec().value());
    EXPECT_FALSE(duplicate.IsCloseOnExec().value());
    EXPECT_TRUE(duplicate.SetCloseOnExec(true).has_value());
    EXPECT_TRUE(duplicate.IsCloseOnExec().value());
}

#endif