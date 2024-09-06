#include <thread>
#include <array>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <zeus/foundation/sync/mutex_object.hpp>
#include <zeus/foundation/sync/latch.h>
#include <zeus/foundation/sync/event.h>
#include <zeus/foundation/sync/condition_variable.h>
#include <zeus/foundation/sync/file_mutex.h>
#include <zeus/foundation/thread/thread_pool.h>
#include <zeus/foundation/time/time.h>
#include "move_test.hpp"

using namespace std::chrono;
using namespace zeus;

TEST(Event, EarlyWake)
{
    const auto DURATION = std::chrono::seconds(1);
    Event      event;
    auto       preTime = std::chrono::steady_clock::now();
    event.Notify();
    event.Wait();
    EXPECT_LE(std::chrono::steady_clock::now() - preTime, std::chrono::milliseconds(100));

    preTime = std::chrono::steady_clock::now();
    event.Notify();
    EXPECT_TRUE(event.WaitTimeout(DURATION));
    EXPECT_LE(std::chrono::steady_clock::now() - preTime, std::chrono::milliseconds(100));

    preTime = std::chrono::steady_clock::now();
    event.Notify();
    EXPECT_TRUE(event.WaitUntil(DURATION + std::chrono::steady_clock::now()));
    EXPECT_LE(std::chrono::steady_clock::now() - preTime, std::chrono::milliseconds(100));
}

TEST(Event, WaitNotify)
{
    Event      event;
    const auto DURATION  = std::chrono::seconds(1);
    const auto DEVIATION = std::chrono::milliseconds(500);
    auto       preTime   = std::chrono::steady_clock::now();
    auto       th        = std::thread(
        [&event, DURATION]()
        {
            std::this_thread::sleep_for(DURATION);
            event.Notify();
        }
    );

    event.Wait();
    auto nowTime = std::chrono::steady_clock::now();

    EXPECT_LE(preTime, nowTime);

    EXPECT_GE(nowTime - preTime, DURATION - DEVIATION);
    EXPECT_LE(nowTime - preTime, DURATION + DEVIATION);
    th.join();
    preTime = std::chrono::steady_clock::now();
    th      = std::thread(
        [&event, DURATION]()
        {
            std::this_thread::sleep_for(DURATION);
            event.Notify();
        }
    );
    event.Wait();
    nowTime = std::chrono::steady_clock::now();
    EXPECT_GE(nowTime - preTime, DURATION - DEVIATION);
    EXPECT_LE(nowTime - preTime, DURATION + DEVIATION);
    th.join();
}

TEST(Event, WaitTimeoutNotify)
{
    Event      event;
    const auto TIME = std::chrono::seconds(2);

    auto preTime = std::chrono::steady_clock::now();
    EXPECT_FALSE(event.WaitTimeout(TIME));
    auto excuteDuration = std::chrono::steady_clock::now() - preTime;
    EXPECT_TRUE(excuteDuration - TIME < TIME / 100 || TIME - excuteDuration < TIME / 100);
    preTime = std::chrono::steady_clock::now();

    const auto DEVIATION = std::chrono::milliseconds(500);
    const auto DURATION  = std::chrono::seconds(1);

    auto th = std::thread(
        [&event, DURATION]()
        {
            std::this_thread::sleep_for(DURATION);
            event.Notify();
        }
    );

    EXPECT_TRUE(event.WaitTimeout(TIME));
    auto nowTime = std::chrono::steady_clock::now();

    EXPECT_LE(preTime, nowTime);

    EXPECT_GE(nowTime - preTime, DURATION - DEVIATION);
    EXPECT_LE(nowTime - preTime, DURATION + DEVIATION);
    th.join();
}

TEST(Event, MultiWaitTimeoutNotify)
{
    Event      event;
    const auto TIME         = std::chrono::seconds(10);
    const auto DEVIATION    = std::chrono::milliseconds(500);
    const int  kSize        = 8;
    bool       flags[kSize] = {false};
    for (int i = 0; i < kSize; i++)
    {
        std::thread t(
            [&event, TIME, &flags, i]()
            {
                event.WaitTimeout(TIME);
                flags[i] = true;
            }
        );
        t.detach();
    }
    std::this_thread::sleep_for(DEVIATION + TIME);
    for (int i = 0; i < kSize; i++)
    {
        EXPECT_TRUE(flags[i]);
    }
    std::this_thread::sleep_for(TIME);
}

TEST(Event, WaitUntilNotify)
{
    Event      event;
    const auto TIME = std::chrono::seconds(2);

    auto preTime = std::chrono::steady_clock::now();
    EXPECT_FALSE(event.WaitUntil(preTime + TIME));
    auto excuteDuration = std::chrono::steady_clock::now() - preTime;

    EXPECT_TRUE(excuteDuration - TIME < TIME / 100 || TIME - excuteDuration < TIME / 100);
    preTime = std::chrono::steady_clock::now();

    const auto DEVIATION = std::chrono::milliseconds(500);
    const auto DURATION  = std::chrono::seconds(1);
    auto       th        = std::thread(
        [&event, DURATION]()
        {
            std::this_thread::sleep_for(DURATION);
            event.Notify();
        }
    );

    EXPECT_TRUE(event.WaitUntil(preTime + TIME));
    auto nowTime = std::chrono::steady_clock::now();

    EXPECT_LE(preTime, nowTime);

    EXPECT_GE(nowTime - preTime, DURATION - DEVIATION);
    EXPECT_LE(nowTime - preTime, DURATION + DEVIATION);
    th.join();
}

TEST(Event, WaitNotifyAll)
{
    Event     event;
    const int WAIT_SIZE = 5;
    auto      fun       = [&event]()
    {
        event.Wait();
    };
    std::thread threads[WAIT_SIZE];
    for (int i = 0; i < WAIT_SIZE; i++)
    {
        threads[i] = std::thread(fun);
    }
    const auto  DURATION = std::chrono::seconds(5);
    auto        preTime  = std::chrono::steady_clock::now();
    std::thread notify(
        [&event, DURATION]()
        {
            std::this_thread::sleep_for(DURATION);
            event.NotifyAll();
        }
    );
    for (int i = 0; i < WAIT_SIZE; i++)
    {
        threads[i].join();
    }
    EXPECT_GE(std::chrono::steady_clock::now() - preTime, DURATION);
    notify.join();
}

TEST(Event, WaitNotifyOne)
{
    const auto                  TIME = std::chrono::seconds(5);
    Event                       event;
    const int                   WAIT_SIZE = 5;
    std::thread                 threads[WAIT_SIZE];
    std::array<bool, WAIT_SIZE> statuses = {false};
    auto                        fun      = [&event, &statuses, &TIME](size_t index)
    {
        event.WaitTimeout(TIME);
        statuses[index] = true;
    };

    for (int i = 0; i < WAIT_SIZE; i++)
    {
        threads[i] = std::thread(fun, i);
    }
    event.Notify();
    std::this_thread::sleep_for(TIME / 2);
    size_t sum = 0;
    for (auto& status : statuses)
    {
        if (status)
        {
            sum++;
        }
    }
    EXPECT_EQ(1, sum);
    for (int i = 0; i < WAIT_SIZE; i++)
    {
        threads[i].join();
    }
}

TEST(Event, Reset)
{
    Event event;
    event.Notify();
    event.Reset();
    EXPECT_FALSE(event.WaitTimeout(std::chrono::milliseconds(10)));
}

TEST(Latch, Base)
{
    static const size_t                   TOTAL = 1000;
    std::atomic<size_t>                   count = 0;
    Latch                                 latch(TOTAL);
    ThreadPool                            pool(6);
    std::chrono::steady_clock::time_point point;
    for (size_t i = 0; i < TOTAL; i++)
    {
        pool.Commit(
            [&latch, &count, &point]()
            {
                if (TOTAL == ++count)
                {
                    EXPECT_FALSE(point.time_since_epoch().count());
                    point = std::chrono::steady_clock::now();
                }
                latch.CountDown();
            }
        );
    }
    latch.Wait();
    EXPECT_EQ(count, TOTAL);
    auto diff = std::chrono::steady_clock::now() - point;
    EXPECT_LT(diff, std::chrono::milliseconds(10));
}

TEST(Latch, Delay)
{
    static const size_t                   TOTAL = 1000;
    std::atomic<size_t>                   count = 0;
    Latch                                 latch(TOTAL);
    ThreadPool                            pool(6);
    std::chrono::steady_clock::time_point point;
    for (size_t i = 0; i < TOTAL; i++)
    {
        pool.Commit(
            [&latch, &count, &point]()
            {
                zeus::Sleep(std::chrono::milliseconds(10));
                if (TOTAL == ++count)
                {
                    EXPECT_FALSE(point.time_since_epoch().count());
                    point = std::chrono::steady_clock::now();
                }
                latch.CountDown();
            }
        );
    }
    latch.Wait();
    EXPECT_EQ(count, TOTAL);
    auto diff = std::chrono::steady_clock::now() - point;
    EXPECT_LT(diff, std::chrono::milliseconds(10));
}

TEST(MutexObject, mutex)
{
    auto const       kDuration = std::chrono::seconds(2);
    auto             now       = std::chrono::steady_clock::now();
    const int        kValue    = 3;
    MutexObject<int> mutexObject(kValue);
    EXPECT_EQ(kValue, mutexObject.Value());

    std::thread t(
        [&kDuration, &mutexObject, kValue]()
        {
            MUTEX_OBJECT_LOCK(mutexObject);
            *mutexObject += kValue;
            std::this_thread::sleep_for(kDuration);
        }
    );

    std::this_thread::sleep_for(kDuration / 2);
    {
        std::unique_lock lock(mutexObject);
        mutexObject.Value() += kValue;
    }
    EXPECT_GT(std::chrono::steady_clock::now() - now, kDuration);
    t.join();
    EXPECT_EQ(kValue + kValue + kValue, mutexObject.Value());
    {
        std::unique_lock lock(mutexObject);
        mutexObject = kValue;
        EXPECT_EQ(kValue, mutexObject.Value());
        *mutexObject = kValue * 2;
        EXPECT_EQ(kValue * 2, mutexObject.Value());
    }
    EXPECT_EQ(kValue * 2, mutexObject.Load());
    mutexObject.Store(kValue);
    EXPECT_EQ(kValue, mutexObject.Load());

    int swap = kValue * 3;
    mutexObject.Swap(swap);
    EXPECT_EQ(kValue * 3, mutexObject.Load());
    EXPECT_EQ(kValue, swap);
}

TEST(MutexObject, recursive_mutex)
{
    auto const                             kDuration = std::chrono::seconds(2);
    auto                                   now       = std::chrono::steady_clock::now();
    const int                              kValue    = 3;
    MutexObject<int, std::recursive_mutex> mutexObject(kValue);
    EXPECT_EQ(kValue, mutexObject.Value());

    std::thread t(
        [&kDuration, &mutexObject, kValue]()
        {
            MUTEX_OBJECT_LOCK(mutexObject);
            std::unique_lock lock(mutexObject);
            MUTEX_OBJECT_LOCK(mutexObject);
            *mutexObject += kValue;
            std::this_thread::sleep_for(kDuration);
        }
    );

    std::this_thread::sleep_for(kDuration / 2);
    {
        MUTEX_OBJECT_LOCK(mutexObject);
        MUTEX_OBJECT_LOCK(mutexObject);
        MUTEX_OBJECT_LOCK(mutexObject);
        mutexObject.Value() += kValue;
    }
    EXPECT_GT(std::chrono::steady_clock::now() - now, kDuration);
    t.join();
    EXPECT_EQ(kValue + kValue + kValue, mutexObject.Value());
    {
        MUTEX_OBJECT_LOCK(mutexObject);
        mutexObject = kValue;
        EXPECT_EQ(kValue, mutexObject.Value());
        *mutexObject = kValue * 2;
        EXPECT_EQ(kValue * 2, mutexObject.Value());
    }
    EXPECT_EQ(kValue * 2, mutexObject.Load());
    mutexObject.Store(kValue);
    EXPECT_EQ(kValue, mutexObject.Load());

    int swap = kValue * 3;
    mutexObject.Swap(swap);
    EXPECT_EQ(kValue * 3, mutexObject.Load());
    EXPECT_EQ(kValue, swap);
}

TEST(MutexObject, base)
{
    {
        const char*              kValue = "test";
        MutexObject<std::string> mutexObject(kValue, strlen(kValue));
        EXPECT_EQ(kValue, mutexObject.Value());
        EXPECT_EQ(kValue, *mutexObject);
    }
    {
        const std::string        kValue("test");
        MutexObject<std::string> mutexObject(kValue);
        EXPECT_EQ(kValue, mutexObject.Value());
        EXPECT_EQ(kValue, *mutexObject);
    }
    {
        const std::string        kBase("test");
        std::string              value(kBase);
        MutexObject<std::string> mutexObject(std::move(value));
        EXPECT_EQ(kBase, mutexObject.Value());
        EXPECT_EQ(kBase, *mutexObject);
        EXPECT_TRUE(value.empty());
    }
    {
        MoveTest              base;
        MutexObject<MoveTest> test(std::move(base));
        EXPECT_TRUE(base.MoveConstruct());
        EXPECT_TRUE(test->MoveConstruct());
        EXPECT_FALSE(base.MoveAssign());
        EXPECT_FALSE(test->MoveAssign());
    }
    {
        MoveTest              base;
        MutexObject<MoveTest> test(base);
        EXPECT_FALSE(base.MoveConstruct());
        EXPECT_FALSE(test->MoveConstruct());
        EXPECT_FALSE(base.MoveAssign());
        EXPECT_FALSE(test->MoveAssign());
    }
    {
        MoveTest              base;
        MutexObject<MoveTest> test;
        test = std::move(base);
        EXPECT_FALSE(base.MoveConstruct());
        EXPECT_FALSE(test->MoveConstruct());
        EXPECT_TRUE(base.MoveAssign());
        EXPECT_TRUE(test->MoveAssign());
    }
    {
        MoveTest              base;
        MutexObject<MoveTest> test;
        test = base;
        EXPECT_FALSE(base.MoveConstruct());
        EXPECT_FALSE(test->MoveConstruct());
        EXPECT_FALSE(base.MoveAssign());
        EXPECT_FALSE(test->MoveAssign());
    }
}

TEST(ConditionVariable, WaitNotify)
{
    ConditionVariable cv;
    const auto        DURATION  = std::chrono::seconds(1);
    const auto        DEVIATION = std::chrono::milliseconds(500);
    auto              preTime   = std::chrono::steady_clock::now();
    bool              flag      = false;
    auto              th        = std::thread(
        [&cv, DURATION, &flag]()
        {
            std::this_thread::sleep_for(DURATION);
            std::unique_lock<ConditionVariable> lock(cv);
            flag = true;
            cv.NotifyOne();
        }
    );
    {
        std::unique_lock<ConditionVariable> lock(cv);
        cv.Wait([&flag]() { return flag; });
    }
    auto nowTime = std::chrono::steady_clock::now();

    EXPECT_LE(preTime, nowTime);

    EXPECT_GE(nowTime - preTime, DURATION - DEVIATION);
    EXPECT_LE(nowTime - preTime, DURATION + DEVIATION);
    th.join();
    preTime = std::chrono::steady_clock::now();
    th      = std::thread(
        [&cv, DURATION, &flag]()
        {
            std::this_thread::sleep_for(DURATION);
            std::unique_lock<ConditionVariable> lock(cv);
            flag = true;
            cv.NotifyOne();
        }
    );
    {
        std::unique_lock<ConditionVariable> lock(cv);
        flag = false;
        cv.Wait([&flag]() { return flag; });
    }
    nowTime = std::chrono::steady_clock::now();
    EXPECT_GE(nowTime - preTime, DURATION - DEVIATION);
    EXPECT_LE(nowTime - preTime, DURATION + DEVIATION);
    th.join();

    preTime = std::chrono::steady_clock::now();
    th      = std::thread(
        [&cv, DURATION, &flag]()
        {
            std::this_thread::sleep_for(DURATION);
            std::unique_lock<ConditionVariable> lock(cv);
            flag = true;
            cv.NotifyOne();
        }
    );
    {
        std::unique_lock<ConditionVariable> lock(cv);
        cv.Wait([&flag]() { return flag; });
    }
    nowTime = std::chrono::steady_clock::now();
    EXPECT_LE(nowTime - preTime, std::chrono::milliseconds(10));
    th.join();
}

TEST(ConditionVariable, WaitTimeoutNotify)
{
    ConditionVariable cv;
    const auto        TIME    = std::chrono::seconds(2);
    bool              flag    = false;
    auto              preTime = std::chrono::steady_clock::now();
    {
        std::unique_lock<ConditionVariable> lock(cv);
        EXPECT_FALSE(cv.WaitTimeout(TIME, [&flag]() { return flag; }));
    }
    auto excuteDuration = std::chrono::steady_clock::now() - preTime;
    EXPECT_TRUE(excuteDuration - TIME < TIME / 100 || TIME - excuteDuration < TIME / 100);
    preTime = std::chrono::steady_clock::now();

    const auto DEVIATION = std::chrono::milliseconds(500);
    const auto DURATION  = std::chrono::seconds(1);

    auto th = std::thread(
        [&cv, DURATION, &flag]()
        {
            std::this_thread::sleep_for(DURATION);
            std::unique_lock<ConditionVariable> lock(cv);
            flag = true;
            cv.NotifyOne();
        }
    );
    {
        std::unique_lock<ConditionVariable> lock(cv);
        EXPECT_TRUE(cv.WaitTimeout(TIME, [&flag]() { return flag; }));
    }

    auto nowTime = std::chrono::steady_clock::now();

    EXPECT_LE(preTime, nowTime);

    EXPECT_GE(nowTime - preTime, DURATION - DEVIATION);
    EXPECT_LE(nowTime - preTime, DURATION + DEVIATION);
    th.join();
}

TEST(ConditionVariable, WaitUntilNotify)
{
    ConditionVariable cv;
    const auto        TIME    = std::chrono::seconds(2);
    bool              flag    = false;
    auto              preTime = std::chrono::steady_clock::now();
    {
        std::unique_lock<ConditionVariable> lock(cv);

        EXPECT_FALSE(cv.WaitUntil(preTime + TIME, [&flag]() { return flag; }));
    }
    auto excuteDuration = std::chrono::steady_clock::now() - preTime;

    EXPECT_TRUE(excuteDuration - TIME < TIME / 100 || TIME - excuteDuration < TIME / 100);
    preTime = std::chrono::steady_clock::now();

    const auto DEVIATION = std::chrono::milliseconds(500);
    const auto DURATION  = std::chrono::seconds(1);
    auto       th        = std::thread(
        [&cv, DURATION, &flag]()
        {
            std::this_thread::sleep_for(DURATION);
            std::unique_lock<ConditionVariable> lock(cv);
            flag = true;
            cv.NotifyOne();
        }
    );
    {
        std::unique_lock<ConditionVariable> lock(cv);
        EXPECT_TRUE(cv.WaitUntil(preTime + TIME, [&flag]() { return flag; }));
    }
    auto nowTime = std::chrono::steady_clock::now();

    EXPECT_LE(preTime, nowTime);

    EXPECT_GE(nowTime - preTime, DURATION - DEVIATION);
    EXPECT_LE(nowTime - preTime, DURATION + DEVIATION);
    th.join();
}

TEST(ConditionVariable, WaitNotifyAll)
{
    ConditionVariable cv;
    const int         WAIT_SIZE = 5;
    bool              flag      = false;
    auto              fun       = [&cv, &flag]()
    {
        std::unique_lock<ConditionVariable> lock(cv);
        cv.Wait([&flag]() { return flag; });
    };
    std::thread threads[WAIT_SIZE];
    for (int i = 0; i < WAIT_SIZE; i++)
    {
        threads[i] = std::thread(fun);
    }
    const auto  DURATION = std::chrono::seconds(5);
    auto        preTime  = std::chrono::steady_clock::now();
    std::thread notify(
        [&cv, DURATION, &flag]()
        {
            std::this_thread::sleep_for(DURATION);
            std::unique_lock<ConditionVariable> lock(cv);
            flag = true;
            cv.NotifyAll();
        }
    );
    for (int i = 0; i < WAIT_SIZE; i++)
    {
        threads[i].join();
    }
    EXPECT_GE(std::chrono::steady_clock::now() - preTime, DURATION);
    notify.join();
}

TEST(ConditionVariable, WaitNotifyOne)
{
    const auto                  TIME = std::chrono::seconds(5);
    ConditionVariable           cv;
    bool                        flag      = false;
    const int                   WAIT_SIZE = 5;
    std::thread                 threads[WAIT_SIZE];
    std::array<bool, WAIT_SIZE> statuses = {false};
    auto                        fun      = [&cv, &statuses, &TIME, &flag](size_t index)
    {
        std::unique_lock<ConditionVariable> lock(cv);
        cv.WaitTimeout(TIME, [&flag]() { return flag; });
        statuses[index] = true;
    };

    for (int i = 0; i < WAIT_SIZE; i++)
    {
        threads[i] = std::thread(fun, i);
    }
    std::this_thread::sleep_for(TIME / 4);
    {
        std::unique_lock<ConditionVariable> lock(cv);
        flag = true;
        cv.NotifyOne();
    }
    std::this_thread::sleep_for(TIME / 2);

    size_t sum = 0;
    for (auto& status : statuses)
    {
        if (status)
        {
            sum++;
        }
    }
    EXPECT_EQ(1, sum);
    for (int i = 0; i < WAIT_SIZE; i++)
    {
        threads[i].join();
    }
}

class TempFile
{
public:
    TempFile(const std::string& name) : _path(std::filesystem::temp_directory_path() / name) { std::ofstream file(_path); }
    ~TempFile() { std::filesystem::remove(_path); }
    operator std::filesystem::path() const { return _path; }
private:
    std::filesystem::path _path;
};

TEST(FileMutex, base)
{
    TempFile file("mutex_gtest");
    auto     mutex = FileMutex::Create(file);
    ASSERT_TRUE(mutex.has_value());
    EXPECT_TRUE(mutex->LockExclusive().has_value());
    EXPECT_FALSE(mutex->TryLockExclusive().value());
    EXPECT_TRUE(mutex->Unlock().has_value());
    EXPECT_TRUE(mutex->TryLockExclusive().value());
    EXPECT_TRUE(mutex->Unlock().has_value());
}

TEST(FileMutex, multiInstance)
{
    TempFile file("mutex_gtest");
    auto     mutex = FileMutex::Create(file);
    ASSERT_TRUE(mutex.has_value());
    EXPECT_TRUE(mutex->TryLockExclusive().value());
    auto mutex1 = FileMutex::Create(file);
    ASSERT_TRUE(mutex1.has_value());
    EXPECT_FALSE(mutex1->TryLockExclusive().value());
    EXPECT_FALSE(mutex1->TryLockShared().value());
    EXPECT_TRUE(mutex->Unlock().has_value());
    EXPECT_TRUE(mutex1->LockShared().has_value());
    EXPECT_TRUE(mutex->TryLockShared().value());
    EXPECT_FALSE(mutex->TryLockExclusive().value());
    EXPECT_TRUE(mutex1->Unlock().has_value());
    EXPECT_TRUE(mutex->Unlock().has_value());
    EXPECT_TRUE(mutex->TryLockExclusive().value());
    EXPECT_TRUE(mutex->Unlock().has_value());
}

TEST(FileMutex, concurrentInstance)
{
    TempFile         file("mutex_gtest");
    static const int count       = 100000;
    static const int threadcount = 10;
    std::thread      threads[threadcount];
    int              sum = 0;
    for (int i = 0; i < threadcount; i++)
    {
        threads[i] = std::thread(
            [&sum, &file]()
            {
                auto mutex = FileMutex::Create(file);
                ASSERT_TRUE(mutex.has_value());
                for (int i = 0; i < count; i++)
                {
                    std::unique_lock<FileMutex> lock(mutex.value());
                    sum++;
                }
            }
        );
    }
    for (int i = 0; i < threadcount; i++)
    {
        threads[i].join();
    }
    EXPECT_EQ(threadcount * count, sum);
}

TEST(FileMutex, concurrentThread)
{
    static const int count       = 100000;
    static const int threadcount = 10;
    std::thread      threads[threadcount];
    int              sum = 0;
    TempFile         file("mutex_gtest");
    auto             mutex = FileMutex::Create(file);
    ASSERT_TRUE(mutex.has_value());
    for (int i = 0; i < threadcount; i++)
    {
        threads[i] = std::thread(
            [&sum, &mutex]()
            {
                for (int i = 0; i < count; i++)
                {
                    std::unique_lock<FileMutex> lock(mutex.value());
                    sum++;
                }
            }
        );
    }
    for (int i = 0; i < threadcount; i++)
    {
        threads[i].join();
    }
    EXPECT_EQ(threadcount * count, sum);
}