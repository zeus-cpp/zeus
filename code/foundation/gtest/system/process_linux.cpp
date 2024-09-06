#ifdef __linux__
#include <set>
#include <map>
#include <cctype>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <gtest/gtest.h>
#include <zeus/foundation/system/current_exe.h>
#include <zeus/foundation/system/process.h>
#include <zeus/foundation/resource/auto_release.h>
#include <zeus/foundation/file/kv_file_utils.h>

using namespace zeus;

TEST(Process, Current)
{
    EXPECT_EQ(getpid(), Process::GetCurrentId());
    EXPECT_EQ(getppid(), Process::GetCurrentProcess().ParentId());
    EXPECT_EQ(getpid(), Process::GetCurrentProcess().Id());
    EXPECT_EQ(CurrentExe::GetAppPath(), Process::GetCurrentProcess().ExePath());
    EXPECT_EQ(CurrentExe::GetAppPathString(), Process::GetCurrentProcess().ExePathString());
    EXPECT_NE(0, Process::GetProcessCreateTimeTick(Process::GetCurrentId()).value());
    EXPECT_EQ(Process::GetProcessCreateTimeTick(Process::GetCurrentId()).value(), Process::GetCurrentProcess().CreateTimeTick());
    EXPECT_GE(
        std::chrono::milliseconds(100), Process::GetProcessCreateTime(Process::GetCurrentId()).value() - Process::GetCurrentProcess().CreateTime()
    );
}

static bool IsNumber(const char* str)
{
    for (; *str && std::isdigit(*str); ++str)
    {
    }
    return *str == '\0';
}

static bool IsZombie(int pid)
{
    return "Z (zombie)" == GetKVFileValue(std::filesystem::path("/proc") / std::to_string(pid) / "status", "State", ":\t");
}

TEST(Process, ListId)
{
    DIR* dir = opendir("/proc");
    ASSERT_TRUE(dir);
    AutoRelease     autoRelease([&dir]() { closedir(dir); });
    std::set<pid_t> expectPids;

    for (const dirent* entry = readdir(dir); entry; entry = readdir(dir))
    {
        // 4 : directory 8 : file
        if (DT_DIR == entry->d_type && IsNumber(entry->d_name) && !IsZombie(std::stol(entry->d_name)))
        {
            expectPids.emplace(std::stol(entry->d_name));
        }
    }

    auto processes = Process::ListId();
    EXPECT_GT(5, std::abs(static_cast<int64_t>(expectPids.size()) - static_cast<int64_t>(processes.size())));

    for (const auto& item : processes)
    {
        if (expectPids.end() == expectPids.find(item))
        {
            EXPECT_TRUE(false);
            break;
        }
    }
}

TEST(Process, ListAll)
{
    DIR* dir = opendir("/proc");
    ASSERT_TRUE(dir);
    AutoRelease           autoRelease([&dir]() { closedir(dir); });
    std::map<pid_t, bool> expectPids;

    for (const dirent* entry = readdir(dir); entry; entry = readdir(dir))
    {
        // 4 : directory 8 : file
        if (DT_DIR == entry->d_type && IsNumber(entry->d_name) && !IsZombie(std::stol(entry->d_name)))
        {
            bool permission = access((std::string("/proc/") + entry->d_name + "/exe").c_str(), R_OK) == 0;
            expectPids.emplace(std::stol(entry->d_name), permission);
        }
    }

    auto processes = Process::ListAll();
    EXPECT_GT(5, std::abs(static_cast<int64_t>(expectPids.size()) - static_cast<int64_t>(processes.size())));
    auto                                  now = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point bootTime;
    timespec                              ts = {};
    if (0 == clock_gettime(CLOCK_BOOTTIME, &ts))
    {
        bootTime = std::chrono::system_clock::now() - std::chrono::seconds(ts.tv_sec) - std::chrono::nanoseconds(ts.tv_nsec);
        bootTime -= std::chrono::milliseconds(10); //clock_gettime不太准，修正一下
    }
    for (const auto& item : processes)
    {
        if (auto iter = expectPids.find(item.Id()); expectPids.end() == iter)
        {
            EXPECT_TRUE(false);
            continue;
        }
        else
        {
            EXPECT_TRUE(!item.Name().empty());
            if (iter->second)
            {
                EXPECT_TRUE(!item.ExePath().empty());
                EXPECT_TRUE(!item.ExePathString().empty());
                EXPECT_EQ(item.ExePath(), std::filesystem::path(item.ExePathString()));
            }
            if (item.Id() > 10)
            {
                EXPECT_NE(0, item.ParentId());
            }
            EXPECT_NE(item.Id(), item.ParentId());
            EXPECT_LE(item.CreateTime(), now);
            EXPECT_GT(item.CreateTime(), bootTime);
        }
    }
}

TEST(Process, EnvironmentVariable)
{
    for (const auto& item : Process::ListAll())
    {
        auto env = Process::GetProcessEnvironmentVariable(item.Id());
    }
}

TEST(Process, Find)
{
    auto targets = Process::FindProcessByName("bash");
    EXPECT_GT(targets.size(), 1);
    for (auto const& item : targets)
    {
        EXPECT_EQ("bash", item.Name());
        auto process = Process::FindProcessById(item.Id());
        EXPECT_TRUE(process.has_value());
        EXPECT_EQ(item.Id(), process->Id());
        EXPECT_EQ("bash", process->Name());
    }
}

TEST(Process, Zombie)
{
    DIR* dir = opendir("/proc");
    ASSERT_TRUE(dir);
    AutoRelease     autoRelease([&dir]() { closedir(dir); });
    std::set<pid_t> zombiePids;

    for (const dirent* entry = readdir(dir); entry; entry = readdir(dir))
    {
        // 4 : directory 8 : file
        if (DT_DIR == entry->d_type && IsNumber(entry->d_name) && IsZombie(std::stol(entry->d_name)))
        {
            zombiePids.emplace(std::stol(entry->d_name));
        }
    }
    for (const auto pid : zombiePids)
    {
        EXPECT_TRUE(Process::IsProcessZombie(pid).has_value());
        EXPECT_TRUE(Process::IsProcessZombie(pid).value());
    }
}

#endif