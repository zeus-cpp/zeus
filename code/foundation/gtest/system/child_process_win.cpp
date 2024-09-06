#ifdef _WIN32
#include <thread>
#include <winerror.h>
#include <gtest/gtest.h>
#include <fmt/format.h>
#include <zeus/foundation/time/time.h>
#include <zeus/foundation/system/child_process.h>

using namespace zeus;

TEST(ChildProcess, pause)
{
    static constexpr size_t errorCode = 1345;
    auto                    child     = zeus::ChildProcessExecutor().EnableShow(false).ExecuteCommand("pause");
    ASSERT_TRUE(child.has_value());
    const auto pid = child->GetPID();
    EXPECT_GT(pid, 0);
    EXPECT_TRUE(!!zeus::Process::FindProcessById(pid));
    auto begin = std::chrono::steady_clock::now();
    auto wait  = child->Wait(std::chrono::seconds(3));
    auto end   = std::chrono::steady_clock::now();
    auto diff  = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    EXPECT_GE(diff, std::chrono::milliseconds(2800));
    EXPECT_FALSE(wait);
    EXPECT_EQ(WAIT_TIMEOUT, wait.error().value());
    std::thread t(
        [&child]()
        {
            zeus::Sleep(std::chrono::seconds(1));
            child->Kill(errorCode);
        }
    );
    begin = std::chrono::steady_clock::now();
    wait  = child->Wait(std::chrono::seconds(2));
    end   = std::chrono::steady_clock::now();
    EXPECT_LT(end - begin, std::chrono::seconds(2));
    EXPECT_TRUE(wait);
    EXPECT_EQ(errorCode, *wait);
    EXPECT_FALSE(!!zeus::Process::FindProcessById(pid));
    t.join();
}

#endif