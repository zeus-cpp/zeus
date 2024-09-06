#include <thread>
#include <mutex>
#include <cstring>
#include <gtest/gtest.h>
#include <zeus/foundation/core/random.h>
#include <zeus/foundation/ipc/global_mutex.h>
#include <zeus/foundation/ipc/shared_memory.h>
#include <zeus/foundation/ipc/global_event.h>
#include <zeus/foundation/sync/event.h>
#include <zeus/foundation/system/process.h>

using namespace zeus;

namespace
{
static const std::string kMutexName = "zeus_global_mutex_test";
static const std::string kEventName = "zeus_global_event_test";
}

TEST(GlobalMutex, base)
{
    auto mutex = GlobalMutex::OpenOrCreate(kMutexName);
    ASSERT_TRUE(mutex.has_value());
    EXPECT_TRUE(mutex->Lock().has_value());
    EXPECT_FALSE(mutex->TryLock().value());
    EXPECT_TRUE(mutex->Unlock().has_value());
    EXPECT_TRUE(mutex->TryLock().value());
    EXPECT_TRUE(mutex->Unlock().has_value());
}

TEST(GlobalMutex, multiInstance)
{
    Event lockEvent;
    Event unlockEvent;
    auto  mutex = GlobalMutex::OpenOrCreate(kMutexName);
    ASSERT_TRUE(mutex.has_value());
    std::thread thead(
        [&mutex, &lockEvent, &unlockEvent]()
        {
            EXPECT_TRUE(mutex->TryLock().value());
            lockEvent.Notify();
            unlockEvent.Wait();
            EXPECT_TRUE(mutex->Unlock().has_value());
            lockEvent.Notify();
        }
    );
    lockEvent.Wait();
    auto mutex1 = GlobalMutex::OpenOrCreate(kMutexName);
    ASSERT_TRUE(mutex1.has_value());
    EXPECT_FALSE(mutex1->TryLock().value());
    unlockEvent.Notify();
    lockEvent.Wait();
    EXPECT_TRUE(mutex1->TryLock().value());
    EXPECT_TRUE(mutex1->Unlock().has_value());
    thead.join();
}

TEST(GlobalMutex, concurrentInstance)
{
#ifdef __linux__
    GlobalMutex::Clear(kMutexName);
#endif
    static const int count       = 100000;
    static const int threadcount = 10;
    std::thread      threads[threadcount];
    int              sum = 0;
    for (int i = 0; i < threadcount; i++)
    {
        threads[i] = std::thread(
            [&sum]()
            {
                auto mutex = GlobalMutex::OpenOrCreate(kMutexName);
                ASSERT_TRUE(mutex.has_value());
                for (int i = 0; i < count; i++)
                {
                    std::unique_lock<GlobalMutex> lock(mutex.value());
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

TEST(GlobalMutex, concurrentThread)
{
    static const int count       = 100000;
    static const int threadcount = 10;
    std::thread      threads[threadcount];
    int              sum   = 0;
    auto             mutex = GlobalMutex::OpenOrCreate(kMutexName);
    ASSERT_TRUE(mutex.has_value());
    for (int i = 0; i < threadcount; i++)
    {
        threads[i] = std::thread(
            [&sum, &mutex]()
            {
                for (int i = 0; i < count; i++)
                {
                    std::unique_lock<GlobalMutex> lock(mutex.value());
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

TEST(SharedMemory, base)
{
    static constexpr size_t  kSize       = 1024;
    static const std::string kSharedName = "zeus_shared_memory_test";
#ifdef __linux__
    SharedMemory::Clear(kSharedName);
#endif

    auto memory = SharedMemory::OpenOrCreate(kSharedName, kSize);
    ASSERT_TRUE(memory.has_value());
    EXPECT_EQ(memory->Name(), kSharedName);
    EXPECT_GE(memory->Size(), kSize);
    auto data    = RandString(kSize);
    auto mapping = memory->Map(kSize);
    ASSERT_TRUE(mapping.has_value());
    EXPECT_EQ(mapping->Size(), kSize);
    std::memcpy(mapping->Data(), data.data(), data.size());

    auto memory1 = SharedMemory::Open(kSharedName);
    ASSERT_TRUE(memory1.has_value());
    EXPECT_EQ(memory1->Name(), kSharedName);
    EXPECT_GE(memory1->Size(), kSize);
    auto mapping1 = memory1->Map(kSize);
    ASSERT_TRUE(mapping1.has_value());
    EXPECT_EQ(mapping1->Size(), kSize);
    EXPECT_EQ(std::memcmp(mapping1->Data(), data.data(), data.size()), 0);

    auto memory2 = SharedMemory::OpenOrCreate(kSharedName, kSize + 10);
    ASSERT_TRUE(memory2.has_value());
    EXPECT_EQ(memory2->Name(), kSharedName);
    EXPECT_GE(memory2->Size(), kSize);
    auto mapping2 = memory2->Map(kSize + 10);
    ASSERT_TRUE(mapping2.has_value());
    EXPECT_EQ(std::memcmp(mapping2->Data(), data.data(), data.size()), 0);
    data = RandString(kSize);
    std::memcpy(mapping2->Data(), data.data(), data.size());
    EXPECT_EQ(std::memcmp(mapping1->Data(), data.data(), data.size()), 0);
    EXPECT_EQ(std::memcmp(mapping->Data(), data.data(), data.size()), 0);
}

TEST(SharedMemory, offset)
{
    static const size_t      kSize       = MemoryMapping::SystemMemoryAlign() * 10;
    static const std::string kSharedName = "zeus_shared_memory_test";
#ifdef __linux__
    SharedMemory::Clear(kSharedName);
#endif
    auto memory = SharedMemory::OpenOrCreate(kSharedName, kSize);
    ASSERT_TRUE(memory.has_value());
    EXPECT_EQ(memory->Name(), kSharedName);
    EXPECT_GE(memory->Size(), kSize);
    auto data    = RandString(kSize);
    auto mapping = memory->Map(kSize);
    ASSERT_TRUE(mapping.has_value());
    EXPECT_EQ(mapping->Size(), kSize);
    std::memcpy(mapping->Data(), data.data(), data.size());

    const auto offset1 = RandUint32(MemoryMapping::SystemMemoryAlign() + 1, kSize - MemoryMapping::SystemMemoryAlign() - 1);
    auto       memory1 = SharedMemory::Open(kSharedName);
    ASSERT_TRUE(memory1.has_value());
    EXPECT_EQ(memory1->Name(), kSharedName);
    EXPECT_GE(memory1->Size(), kSize);
    auto mapping1 = memory1->Map(kSize - offset1, offset1);
    ASSERT_TRUE(mapping1.has_value());
    EXPECT_EQ(mapping1->Size(), kSize - offset1);
    EXPECT_EQ(std::memcmp(mapping1->Data(), data.data() + offset1, data.size() - offset1), 0);

    const auto offset2 = RandUint32(MemoryMapping::SystemMemoryAlign() * 3 + 1, kSize - MemoryMapping::SystemMemoryAlign() - 1);
    const auto mapSize = kSize - offset2 - RandUint32(1, MemoryMapping::SystemMemoryAlign());
    auto       memory2 = SharedMemory::OpenOrCreate(kSharedName, kSize + 10);
    ASSERT_TRUE(memory2.has_value());
    EXPECT_EQ(memory2->Name(), kSharedName);
    EXPECT_GE(memory2->Size(), kSize);
    auto mapping2 = memory2->Map(mapSize, offset2);
    ASSERT_TRUE(mapping2.has_value());
    EXPECT_EQ(mapping2->Size(), mapSize);
    EXPECT_EQ(std::memcmp(mapping2->Data(), data.data() + offset2, mapSize), 0);
    data = RandString(mapSize);
    std::memcpy(mapping2->Data(), data.data(), data.size());
    EXPECT_EQ(std::memcmp(static_cast<char*>(mapping->Data()) + offset2, data.data(), data.size()), 0);
}

TEST(GlobalEvent, WaitNotify)
{
    auto event = GlobalEvent::OpenOrCreate(kEventName, false);
    ASSERT_TRUE(event.has_value());
#ifdef __linux__
    EXPECT_EQ(Process::GetCurrentId(), event->AliveCreator().value());
#endif
    const auto DURATION  = std::chrono::seconds(1);
    const auto DEVIATION = std::chrono::milliseconds(500);
    auto       preTime   = std::chrono::steady_clock::now();
    auto       th        = std::thread(
        [&event, DURATION]()
        {
            std::this_thread::sleep_for(DURATION);
            event->Set();
        }
    );

    EXPECT_TRUE(event->Wait().has_value());
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
            event->Set();
        }
    );
    EXPECT_TRUE(event->Wait().has_value());
    nowTime = std::chrono::steady_clock::now();
    EXPECT_GE(nowTime - preTime, DURATION - DEVIATION);
    EXPECT_LE(nowTime - preTime, DURATION + DEVIATION);
    th.join();
#ifdef __linux__
    GlobalEvent::Clear(kEventName);
#endif
}

TEST(GlobalEvent, WaitTimeoutNotify)
{
    auto event = GlobalEvent::OpenOrCreate(kEventName, false);
    ASSERT_TRUE(event.has_value());

#ifdef __linux__
    EXPECT_EQ(Process::GetCurrentId(), event->AliveCreator().value());
#endif
    const auto TIME = std::chrono::seconds(2);

    auto preTime = std::chrono::steady_clock::now();
    EXPECT_FALSE(event->WaitTimeout(TIME).value());
    auto excuteDuration = std::chrono::steady_clock::now() - preTime;
    EXPECT_TRUE(excuteDuration - TIME < TIME / 100 || TIME - excuteDuration < TIME / 100);
    preTime = std::chrono::steady_clock::now();

    const auto DEVIATION = std::chrono::milliseconds(500);
    const auto DURATION  = std::chrono::seconds(1);

    auto th = std::thread(
        [&event, DURATION]()
        {
            std::this_thread::sleep_for(DURATION);
            event->Set();
        }
    );

    EXPECT_TRUE(event->WaitTimeout(TIME).value());
    auto nowTime = std::chrono::steady_clock::now();

    EXPECT_LE(preTime, nowTime);

    EXPECT_GE(nowTime - preTime, DURATION - DEVIATION);
    EXPECT_LE(nowTime - preTime, DURATION + DEVIATION);
    th.join();
#ifdef __linux__
    GlobalEvent::Clear(kEventName);
#endif
}

TEST(GlobalEvent, MultiWaitTimeoutNotify)
{
    auto event = GlobalEvent::OpenOrCreate(kEventName, false);
    ASSERT_TRUE(event.has_value());

#ifdef __linux__
    EXPECT_EQ(Process::GetCurrentId(), event->AliveCreator().value());
#endif
    const auto TIME         = std::chrono::seconds(10);
    const auto DEVIATION    = std::chrono::milliseconds(500);
    const int  kSize        = 8;
    bool       flags[kSize] = {false};
    for (int i = 0; i < kSize; i++)
    {
        std::thread t(
            [&event, TIME, &flags, i]()
            {
                EXPECT_FALSE(event->WaitTimeout(TIME).value());
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
#ifdef __linux__
    GlobalEvent::Clear(kEventName);
#endif
}

TEST(GlobalEvent, Manual)
{
    auto event = GlobalEvent::OpenOrCreate(kEventName, true);
    ASSERT_TRUE(event.has_value());

#ifdef __linux__
    EXPECT_EQ(Process::GetCurrentId(), event->AliveCreator().value());
#endif
    const auto DURATION  = std::chrono::seconds(1);
    const auto DEVIATION = std::chrono::milliseconds(500);
    auto       preTime   = std::chrono::steady_clock::now();
    auto       th        = std::thread(
        [&event, DURATION]()
        {
            std::this_thread::sleep_for(DURATION);
            event->Set();
        }
    );

    EXPECT_TRUE(event->Wait().has_value());
    EXPECT_TRUE(event->Wait().has_value());
    EXPECT_TRUE(event->Wait().has_value());
    auto nowTime = std::chrono::steady_clock::now();

    EXPECT_LE(preTime, nowTime);

    EXPECT_GE(nowTime - preTime, DURATION - DEVIATION);
    EXPECT_LE(nowTime - preTime, DURATION + DEVIATION);
    th.join();
    event->Reset();
    preTime = std::chrono::steady_clock::now();
    th      = std::thread(
        [&event, DURATION]()
        {
            std::this_thread::sleep_for(DURATION);
            event->Set();
        }
    );
    EXPECT_TRUE(event->Wait().has_value());
    EXPECT_TRUE(event->Wait().has_value());
    EXPECT_TRUE(event->Wait().has_value());
    nowTime = std::chrono::steady_clock::now();
    EXPECT_GE(nowTime - preTime, DURATION - DEVIATION);
    EXPECT_LE(nowTime - preTime, DURATION + DEVIATION);
    th.join();
#ifdef __linux__
    GlobalEvent::Clear(kEventName);
#endif
}
