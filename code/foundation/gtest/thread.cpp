#include <cstring>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <gtest/gtest.h>
#include <zeus/foundation/thread/thread_pool.h>
#include <zeus/foundation/thread/advanced_thread.h>
#include <zeus/foundation/thread/thread_checker.h>
#include <zeus/foundation/thread/thread_utils.h>
#include <zeus/foundation/sync/latch.h>
#include <zeus/foundation/sync/event.h>
#include <zeus/foundation/sync/mutex_object.hpp>
#include <zeus/foundation/time/time.h>

using namespace std;
using namespace zeus;

TEST(ThreadChecker, check)
{
    {
        ThreadChecker check;
        EXPECT_TRUE(check.IsCurrent());
        check.Detach();
        EXPECT_TRUE(check.IsCurrent());
        EXPECT_TRUE(check.IsCurrent());
        std::thread t(
            [&check]()
            {
                EXPECT_FALSE(check.IsCurrent());
                check.Detach();
                EXPECT_TRUE(check.IsCurrent());
                EXPECT_TRUE(check.IsCurrent());
            }
        );
        t.join();
        EXPECT_FALSE(check.IsCurrent());
        check.Detach();
        EXPECT_TRUE(check.IsCurrent());
        EXPECT_TRUE(check.IsCurrent());
    }
    {
        ThreadChecker check(false);
        std::thread   t(
            [&check]()
            {
                EXPECT_TRUE(check.IsCurrent());
                check.Detach();
                EXPECT_TRUE(check.IsCurrent());
                EXPECT_TRUE(check.IsCurrent());
            }
        );
        t.join();
        EXPECT_FALSE(check.IsCurrent());
        check.Detach();
        EXPECT_TRUE(check.IsCurrent());
        EXPECT_TRUE(check.IsCurrent());
    }
}

TEST(ThreadPool, base)
{
    const int size        = 1000;
    bool      flags[size] = {false};
    auto      func        = [&flags](int index)
    {
        EXPECT_FALSE(flags[index]);
        flags[index] = true;
    };
    ThreadPool  pool(1, false);
    atomic<int> count(0);
    for (int i = 0; i < 5; i++)
    {
        thread(
            [&count, &pool, &func, size]()
            {
                //会跳过1开始
                for (;;)
                {
                    int current = count.fetch_add(1);
                    if (current < size)
                    {
                        pool.CommitTask(bind(func, current));
                    }
                    else
                    {
                        return;
                    }
                }
            }
        ).detach();
    }

    Sleep(chrono::seconds(5));
    bool result = true;
    for (int i = 0; i < size; i++)
    {
        if (!flags[i])
        {
            result = false;
        }
    }
    EXPECT_TRUE(result);
}

TEST(ThreadPool, TaskBlockQueueSize)
{
    const int size        = 1000;
    bool      flags[size] = {false};
    auto      func        = [&flags](int index)
    {
        EXPECT_FALSE(flags[index]);
        flags[index] = true;
        Sleep(chrono::milliseconds(10));
    };
    ThreadPool pool(1, true);
    pool.SetTaskBlockQueueSize(0);
    atomic<int> count(0);
    for (int i = 0; i < 5; i++)
    {
        thread(
            [&count, &pool, &func, size]()
            {
                //会跳过1开始
                for (;;)
                {
                    int current = count.fetch_add(1);
                    if (current < size)
                    {
                        pool.CommitTask(bind(func, current));
                    }
                    else
                    {
                        return;
                    }
                }
            }
        ).detach();
    }

    Sleep(chrono::seconds(15));
    bool result = true;
    for (int i = 0; i < size; i++)
    {
        if (!flags[i])
        {
            result = false;
        }
    }
    EXPECT_TRUE(result);
}

TEST(ThreadPool, SingleCore)
{
    const int size        = 1000;
    bool      flags[size] = {false};
    auto      func        = [&flags](int index)
    {
        EXPECT_FALSE(flags[index]);
        flags[index] = true;
        Sleep(chrono::milliseconds(100));
    };
    ThreadPool  pool(1, false);
    atomic<int> count(0);
    for (int i = 0; i < 5; i++)
    {
        thread(
            [&count, &pool, &func, size]()
            {
                for (;;)
                {
                    int current = count.fetch_add(1);
                    if (current < size)
                    {
                        pool.CommitTask(bind(func, current));
                    }
                    else
                    {
                        return;
                    }
                }
            }
        ).detach();
    }

    Sleep(chrono::seconds(99));
    //正常如果只是用1条线程，一定没办法把1000个状态置为true
    {
        bool result = true;
        for (int i = 0; i < size; i++)
        {
            if (!flags[i])
            {
                result = false;
            }
        }
        EXPECT_FALSE(result);
    }
}

TEST(ThreadPool, LimitCore)
{
    const int size        = 1000;
    bool      flags[size] = {false};
    auto      func        = [&flags](int index)
    {
        EXPECT_FALSE(flags[index]);
        flags[index] = true;
        Sleep(chrono::milliseconds(100));
    };
    ThreadPool  pool(3, false);
    atomic<int> count(0);
    for (int i = 0; i < 5; i++)
    {
        thread(
            [&count, &pool, &func, size]()
            {
                //会跳过1开始
                for (;;)
                {
                    int current = count.fetch_add(1);
                    if (current < size)
                    {
                        pool.CommitTask(bind(func, current));
                    }
                    else
                    {
                        return;
                    }
                }
            }
        ).detach();
    }

    Sleep(chrono::seconds(33));
    {
        bool result = true;
        for (int i = 0; i < size; i++)
        {
            if (!flags[i])
            {
                result = false;
            }
        }
        EXPECT_FALSE(result);
    }

    Sleep(chrono::seconds(9));
    {
        bool result = true;
        for (int i = 0; i < size; i++)
        {
            if (!flags[i])
            {
                result = false;
            }
        }
        EXPECT_TRUE(result);
    }
}

TEST(ThreadPool, ZeroCore)
{
    ThreadPool pool(0, true);
    uint64_t   tid;
    Event      event;

    pool.CommitTask(
        [&tid, &event]()
        {
            tid = GetThreadId();
            event.Notify();
        }
    );
    event.Wait();
    pool.CommitTask(
        [&tid, &event]()
        {
            EXPECT_EQ(tid, GetThreadId());
            event.Notify();
        }
    );
    event.Wait();
    pool.CommitTask(
        [&tid, &event]()
        {
            EXPECT_EQ(tid, GetThreadId());
            event.Notify();
        }
    );
    event.Wait();
    Sleep(std::chrono::minutes(1) + std::chrono::seconds(2));
    pool.CommitTask(
        [&tid, &event]()
        {
            EXPECT_NE(tid, GetThreadId());
            event.Notify();
        }
    );
    event.Wait();
}

TEST(ThreadPool, AutoExpansion)
{
    const int                            size        = 1000;
    const size_t                         threadCount = 4;
    bool                                 flags[size] = {false};
    MutexObject<std::set<std::uint64_t>> threads;
    auto                                 func = [&flags, &threads](int index)
    {
        threads->insert(GetThreadId());
        EXPECT_FALSE(flags[index]);
        flags[index] = true;
        Sleep(chrono::milliseconds(100));
    };
    ThreadPool pool(1, true, threadCount);

    uint64_t tid;
    Event    event;
    pool.Commit(
        [&tid, &event]()
        {
            tid = GetThreadId();
            event.Notify();
        }
    );
    event.Wait();

    atomic<int> count(0);
    for (int i = 0; i < 5; i++)
    {
        thread(
            [&count, &pool, &func, size]()
            {
                //会跳过1开始
                for (;;)
                {
                    int current = count.fetch_add(1);
                    if (current < size)
                    {
                        pool.CommitTask(bind(func, current));
                    }
                    else
                    {
                        return;
                    }
                }
            }
        ).detach();
    }

    Sleep(chrono::milliseconds(100) * size / threadCount + chrono::seconds(5));

    bool result = true;
    for (int i = 0; i < size; i++)
    {
        if (!flags[i])
        {
            result = false;
        }
    }
    EXPECT_TRUE(result);

    Sleep(chrono::minutes(1) * (threadCount - 1) + chrono::seconds(1));

    pool.Commit(
        [&tid, &event, &pool]()
        {
            EXPECT_EQ(tid, GetThreadId());
            pool.Commit(
                [&tid, &event]()
                {
                    EXPECT_EQ(tid, GetThreadId());
                    event.Notify();
                }
            );
        }
    );
    event.Wait();
    EXPECT_EQ(threadCount, threads->size());
}

TEST(ThreadPool, StopClear)
{
    const int size        = 1000;
    bool      flags[size] = {false};
    auto      func        = [&flags](int index)
    {
        EXPECT_FALSE(flags[index]);
        flags[index] = true;
        Sleep(chrono::milliseconds(100));
    };
    ThreadPool  pool(1, true);
    atomic<int> count(0);
    for (int i = 0; i < 5; i++)
    {
        thread(
            [&count, &pool, &func, size]()
            {
                //会跳过1开始
                for (;;)
                {
                    int current = count.fetch_add(1);
                    if (current < size)
                    {
                        pool.CommitTask(bind(func, current));
                    }
                    else
                    {
                        return;
                    }
                }
            }
        ).detach();
    }
    Sleep(chrono::seconds(1));
    pool.Stop();
    size_t setCount = 0;
    for (int i = 0; i < size; i++)
    {
        if (flags[i])
        {
            setCount++;
        }
    }
    EXPECT_GT(setCount, 0);
    EXPECT_LT(setCount, size);
    Sleep(chrono::seconds(1));
    size_t setCountCheck = 0;
    for (int i = 0; i < size; i++)
    {
        if (flags[i])
        {
            setCountCheck++;
        }
    }
    EXPECT_EQ(setCountCheck, setCount);
    std::memset(flags, 0, sizeof(flags));
    count = 0;
    for (int i = 0; i < 5; i++)
    {
        thread(
            [&count, &pool, &func, size]()
            {
                //会跳过1开始
                for (;;)
                {
                    int current = count.fetch_add(1);
                    if (current < size)
                    {
                        pool.CommitTask(bind(func, current));
                    }
                    else
                    {
                        return;
                    }
                }
            }
        ).detach();
    }
    int testcount = thread::hardware_concurrency();

    Sleep(chrono::milliseconds(static_cast<int>(100000.0 / std::max(1, testcount - 2))));
    bool result = true;
    for (int i = 0; i < size; i++)
    {
        if (!flags[i])
        {
            result = false;
        }
    }
    EXPECT_TRUE(result);
}

TEST(ThreadPool, Manual)
{
    const int    size        = 1000;
    const size_t delay       = 20;
    bool         flags[size] = {false};
    auto         func        = [&flags, delay](int index)
    {
        EXPECT_FALSE(flags[index]);
        flags[index] = true;
        Sleep(chrono::milliseconds(delay));
    };
    ThreadPool  pool(1, true, std::thread::hardware_concurrency(), false);
    atomic<int> count(0);
    for (int i = 0; i < 5; i++)
    {
        thread(
            [&count, &pool, &func, size]()
            {
                //会跳过1开始
                for (;;)
                {
                    int current = count.fetch_add(1);
                    if (current < size)
                    {
                        pool.CommitTask(bind(func, current));
                    }
                    else
                    {
                        return;
                    }
                }
            }
        ).detach();
    }
    Sleep(chrono::seconds(1));
    for (int i = 0; i < size; i++)
    {
        EXPECT_FALSE(flags[i]);
    }
    pool.Start();
    Sleep(std::chrono::milliseconds(size * (delay * 2)));
    pool.Stop();
    bool result = true;
    for (int i = 0; i < size; i++)
    {
        if (!flags[i])
        {
            result = false;
        }
    }
    EXPECT_TRUE(result);
}

TEST(ThreadPool, ManualExpansion)
{
    const int                            size  = 1000;
    const size_t                         delay = 20;
    MutexObject<std::set<std::uint64_t>> threads;
    bool                                 flags[size] = {false};
    auto                                 func        = [&flags, delay, &threads](int index)
    {
        threads->insert(GetThreadId());
        EXPECT_FALSE(flags[index]);
        flags[index] = true;
        Sleep(chrono::milliseconds(delay));
    };
    ThreadPool pool(1, true, std::thread::hardware_concurrency(), false);
    pool.Start();
    atomic<int> count(0);
    for (int i = 0; i < 5; i++)
    {
        thread(
            [&count, &pool, &func, size]()
            {
                //会跳过1开始
                for (;;)
                {
                    int current = count.fetch_add(1);
                    if (current < size)
                    {
                        pool.CommitTask(bind(func, current));
                    }
                    else
                    {
                        return;
                    }
                }
            }
        ).detach();
    }

    Sleep(std::chrono::milliseconds(size * delay / std::thread::hardware_concurrency()) + 2s);
    pool.Stop();
    bool result = true;
    for (int i = 0; i < size; i++)
    {
        if (!flags[i])
        {
            result = false;
        }
    }
    EXPECT_TRUE(result);
    EXPECT_EQ(std::thread::hardware_concurrency(), threads->size());
}

TEST(ThreadPool, Restart)
{
    const int size        = 1000;
    bool      flags[size] = {false};
    auto      func        = [&flags](int index)
    {
        EXPECT_FALSE(flags[index]);
        flags[index] = true;
        Sleep(chrono::milliseconds(10));
    };
    ThreadPool  pool(1, true);
    atomic<int> count(0);
    for (int i = 0; i < 5; i++)
    {
        thread(
            [&count, &pool, &func, size]()
            {
                //会跳过1开始
                for (;;)
                {
                    int current = count.fetch_add(1);
                    if (current < size)
                    {
                        pool.CommitTask(bind(func, current));
                    }
                    else
                    {
                        return;
                    }
                }
            }
        ).detach();
    }
    Sleep(chrono::milliseconds(500));
    pool.Stop();
    size_t setCount = 0;
    for (int i = 0; i < size; i++)
    {
        if (flags[i])
        {
            setCount++;
        }
    }
    EXPECT_GT(setCount, 0);
    EXPECT_LT(setCount, size);
    Sleep(chrono::seconds(1));
    size_t setCountCheck = 0;
    for (int i = 0; i < size; i++)
    {
        if (flags[i])
        {
            setCountCheck++;
        }
    }
    EXPECT_EQ(setCountCheck, setCount);
    std::memset(flags, 0, sizeof(flags));
    count = 0;
    for (int i = 0; i < 5; i++)
    {
        thread(
            [&count, &pool, &func, size]()
            {
                //会跳过1开始
                for (;;)
                {
                    int current = count.fetch_add(1);
                    if (current < size)
                    {
                        pool.CommitTask(bind(func, current));
                    }
                    else
                    {
                        return;
                    }
                }
            }
        ).detach();
    }
    int testcount = thread::hardware_concurrency();

    Sleep(chrono::milliseconds(static_cast<int>(100000.0 / std::max(1, testcount - 2))));
    pool.Stop();
    bool result = true;
    for (int i = 0; i < size; i++)
    {
        if (!flags[i])
        {
            result = false;
        }
    }
    EXPECT_TRUE(result);
}

TEST(ThreadPool, InPool)
{
    ThreadPool pool(std::thread::hardware_concurrency(), true);
    pool.SetTaskBlockQueueSize(0);
    std::atomic<size_t> count = 0;
    for (unsigned int i = 0; i < std::thread::hardware_concurrency(); i++)
    {
        pool.CommitTask(
            [&pool, &count]()
            {
                Sleep(1s);
                EXPECT_TRUE(pool.IsPoolThread());
                count++;
            }
        );
    }

    EXPECT_FALSE(pool.IsPoolThread());
    Sleep(std::chrono::seconds(std::thread::hardware_concurrency()));
    EXPECT_GT(count, 0);
}

static void RecursionPoolTask(ThreadPool* pool)
{
    pool->CommitTask(std::bind(&RecursionPoolTask, pool));
}

TEST(ThreadPool, DeadLock)
{
    ThreadPool pool;
    RecursionPoolTask(&pool);
    Sleep(std::chrono::seconds(3));
    pool.Stop();
}

TEST(AdvancedThread, Base)
{
    constexpr int  TEST = 100;
    AdvancedThread thread("test");
    thread.Start();
    bool                  flag  = false;
    int                   count = 0;
    std::function<void()> fun   = [&flag]()
    {
        EXPECT_FALSE(flag);
        flag = true;
    };
    thread.Invoke(fun);
    EXPECT_TRUE(flag);
    EXPECT_TRUE(thread.IsRunning());
    flag = false;
    for (auto i = 0; i < TEST; i++)
    {
        thread.Post([&count]() { ++count; });
    }
    thread.Post([&flag]() { flag = true; });
    auto expectCount = thread.Invoke<int>([&count]() { return count; });
    EXPECT_TRUE(flag);
    EXPECT_EQ(TEST, expectCount);
    flag = false;
    thread.Post(fun);
    thread.Invoke([]() {});
    EXPECT_TRUE(flag);
    flag                               = false;
    std::function<void()> recursionFun = [&fun, &thread]()
    {
        thread.Invoke([&fun, &thread]() { thread.Invoke(fun); });
    };
    thread.Invoke(recursionFun);
    EXPECT_TRUE(flag);
    thread.Stop();
    for (auto i = 0; i < TEST; i++)
    {
        thread.Post([&count]() { ++count; });
    }
}

TEST(AdvancedThread, Concurrent)
{
    const size_t   kTestCount = 100000;
    zeus::Latch    latch(kTestCount);
    AdvancedThread thread("test");
    thread.Start();
    std::atomic<size_t> executeCount = 0;
    ThreadPool          pool(10, false);
    for (size_t index = 0; index < kTestCount; index++)
    {
        pool.CommitTask(
            [&executeCount, &latch, &thread]()
            {
                thread.Invoke([&executeCount]() { executeCount++; });
                latch.CountDown();
            }
        );
    }
    latch.Wait();
    EXPECT_EQ(kTestCount, executeCount);
    thread.Stop();
}

TEST(AdvancedThread, Automatic)
{
    const size_t   kTestCount = 100000;
    zeus::Latch    latch(kTestCount);
    AdvancedThread thread("test", true);

    std::atomic<size_t> executeCount = 0;
    ThreadPool          pool(10, false);
    for (size_t index = 0; index < kTestCount; index++)
    {
        pool.CommitTask(
            [&executeCount, &latch, &thread]()
            {
                thread.Invoke([&executeCount]() { executeCount++; });
                latch.CountDown();
            }
        );
    }
    latch.Wait();
    EXPECT_EQ(kTestCount, executeCount);
}

static void RecursionThreadTask(AdvancedThread* thread)
{
    thread->Post(std::bind(&RecursionThreadTask, thread));
}

TEST(AdvancedThread, DeadLock)
{
    AdvancedThread thread("test", true);
    RecursionThreadTask(&thread);
    Sleep(std::chrono::seconds(3));
    thread.Stop();
}