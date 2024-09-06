#include <thread>
#include <gtest/gtest.h>
#include <zeus/foundation/system/environment.h>

using namespace zeus;

TEST(Environment, PathString)
{
    EXPECT_EQ(Environment::GetHomeString(), Environment::GetHome().u8string());
}

TEST(Environment, ProcessorCount)
{
    EXPECT_EQ(std::thread::hardware_concurrency(), Environment::ProcessorCount());
    EXPECT_LE(Environment::ProcessorCoreCount(), Environment::ProcessorCount());
}
