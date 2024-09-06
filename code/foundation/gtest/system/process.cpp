#include <map>
#include <gtest/gtest.h>
#include <zeus/foundation/system/current_exe.h>
#include <zeus/foundation/system/process.h>
#include <zeus/foundation/time/time_utils.h>
using namespace zeus;

TEST(Process, Tick)
{
    std::map<Process::PID, uint64_t> ticks;
    ASSERT_GT(Process::ListAll().size(), 50);
    for (const auto& process : Process::ListAll())
    {
        if (process.Id() > 1000)
        {
            EXPECT_NE(0, process.CreateTimeTick());
        }
        ticks[process.Id()] = process.CreateTimeTick();
    }

    for (const auto& process : Process::ListAll())
    {
        if (auto iter = ticks.find(process.Id()); iter != ticks.end())
        {
            EXPECT_EQ(iter->second, process.CreateTimeTick());
        }
    }
}

TEST(Process, Map)
{
    auto map = Process::ListMap();
    ASSERT_GT(map.size(), 50);
    for (const auto& [pid, process] : map)
    {
        EXPECT_EQ(pid, process.Id());
    }
}