#ifdef __linux__
#include <thread>
#include <errno.h>
#include <gtest/gtest.h>
#include <fmt/format.h>
#include <zeus/foundation/time/time.h>
#include <zeus/foundation/string/string_utils.h>
#include <zeus/foundation/system/child_process.h>
#include <zeus/foundation/system/current_exe.h>

using namespace zeus;

TEST(ChildProcess, pause)
{
    auto child = zeus::ChildProcessExecutor().ExecuteCommand("read -n1 -r key");
    ASSERT_TRUE(child.has_value());
    const auto pid = child->GetPID();
    EXPECT_GT(pid, 0);
    EXPECT_TRUE(!!zeus::Process::FindProcessById(pid));
    auto begin = std::chrono::steady_clock::now();
    auto wait  = child->Wait(std::chrono::seconds(3));
    auto end   = std::chrono::steady_clock::now();
    auto diff  = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    EXPECT_GE(diff, std::chrono::milliseconds(2800));
    ASSERT_FALSE(wait);
    EXPECT_EQ(ETIMEDOUT, wait.error().value());
    std::thread t(
        [&child]()
        {
            zeus::Sleep(std::chrono::seconds(1));
            child->Kill();
        }
    );
    begin = std::chrono::steady_clock::now();
    wait  = child->Wait(std::chrono::seconds(2));
    end   = std::chrono::steady_clock::now();
    EXPECT_LT(end - begin, std::chrono::seconds(2));
    ASSERT_TRUE(wait);
    EXPECT_EQ(0, *wait);
    EXPECT_FALSE(!!zeus::Process::FindProcessById(pid));
    t.join();
}

TEST(ChildProcess, user)
{
    if (!CurrentExe::HasRootPermission())
    {
        return;
    }
    {
        auto child = zeus::ChildProcessExecutor().EnableRedirectStdout(true).ExecuteCommand("whoami");
        ASSERT_TRUE(child.has_value());
        const auto pid = child->GetPID();
        EXPECT_GT(pid, 0);
        EXPECT_TRUE(!!zeus::Process::FindProcessById(pid));
        EXPECT_FALSE(child->HasStdin());
        EXPECT_FALSE(child->HasStderr());
        ASSERT_TRUE(child->HasStdout());
        std::string out;
        child->GetStdout() >> out;
        EXPECT_EQ("root", TrimEnd(out));
        child->Wait(std::chrono::seconds(0));
    }
    std::string username;
    {
        auto child = zeus::ChildProcessExecutor().EnableRedirectStdout(true).ExecuteCommand(R"(grep 1000 /etc/passwd | cut -d: -f1)");
        ASSERT_TRUE(child.has_value());
        const auto pid = child->GetPID();
        EXPECT_GT(pid, 0);
        EXPECT_TRUE(!!zeus::Process::FindProcessById(pid));
        EXPECT_FALSE(child->HasStdin());
        EXPECT_FALSE(child->HasStderr());
        ASSERT_TRUE(child->HasStdout());
        child->GetStdout() >> username;
        username = TrimEnd(username);
        child->Wait(std::chrono::seconds(0));
        EXPECT_FALSE(username.empty());
    }
    {
        auto child = zeus::ChildProcessExecutor().EnableRedirectStdout(true).SetUID(1000).SetGID(1000).ExecuteCommand("whoami");
        ASSERT_TRUE(child.has_value());
        const auto pid = child->GetPID();
        EXPECT_GT(pid, 0);
        EXPECT_TRUE(!!zeus::Process::FindProcessById(pid));
        EXPECT_FALSE(child->HasStdin());
        EXPECT_FALSE(child->HasStderr());
        ASSERT_TRUE(child->HasStdout());
        std::string out;
        child->GetStdout() >> out;
        EXPECT_EQ(username, TrimEnd(out));
        child->Wait(std::chrono::seconds(0));
    }

    {
        auto child =
            zeus::ChildProcessExecutor().EnableRedirectStdout(true).SetUID(1000).SetGID(1000).ExecuteCommand(R"(sed -n '9,10p' /proc/self/status)");
        ASSERT_TRUE(child.has_value());
        const auto pid = child->GetPID();
        EXPECT_GT(pid, 0);
        EXPECT_TRUE(!!zeus::Process::FindProcessById(pid));
        EXPECT_FALSE(child->HasStdin());
        EXPECT_FALSE(child->HasStderr());
        ASSERT_TRUE(child->HasStdout());
        std::string uidOut;
        std::getline(child->GetStdout(), uidOut);
        std::string gidOut;
        std::getline(child->GetStdout(), gidOut);
        EXPECT_EQ("Uid:\t1000\t1000\t1000\t1000", uidOut);
        EXPECT_EQ("Gid:\t1000\t1000\t1000\t1000", gidOut);
        child->Wait(std::chrono::seconds(0));
    }
}

#endif