
#include <chrono>
#ifdef _WIN32
#include <Windows.h>
#endif
#ifdef __linux__
#include <unistd.h>
#include <sys/eventfd.h>
#endif
#include <gtest/gtest.h>
#include <zeus/foundation/core/platform_def.h>
#include <zeus/foundation/core/posix/eintr_wrapper.h>
#include <zeus/foundation/sync/linux/file_descriptor_set_notify.h>
#include <zeus/foundation/sync/win/handle_set_notify.h>
#include <zeus/foundation/time/time.h>

using namespace zeus;

const auto kDeviation = std::chrono::milliseconds(100);

PlatformFileHandleWrapper CreatePlatformEvent()
{
#ifdef _WIN32
    return PlatformFileHandleWrapper(CreateEventW(nullptr, FALSE, FALSE, nullptr));
#endif
#ifdef __linux__
    return PlatformFileHandleWrapper(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC));
#endif
}

void PulsePlatformEvent(const PlatformFileHandleWrapper& event)
{
#ifdef _WIN32
    SetEvent(event.NativeHandle());
#endif
#ifdef __linux__
    uint64_t value = 1;
    HANDLE_EINTR(write(event.NativeHandle(), &value, sizeof(value)));
#endif
}

void ResetPlatformEvent(const PlatformFileHandleWrapper& event)
{
#ifdef _WIN32
    ResetEvent(event.NativeHandle());
#endif
#ifdef __linux__
    uint64_t value;
    HANDLE_EINTR(read(event.NativeHandle(), &value, sizeof(value)));
#endif
}

#ifdef _WIN32
#define NotifyManager        WinHandleSetNotify
#define AddNotifyCallback    AddHandleStateCallback
#define RemoveNotifyCallback RemoveHandleStateCallback
#endif
#ifdef __linux__
#define NotifyManager        LinuxFileDescriptorSetNotify
#define AddNotifyCallback    AddFileDescriptorReadableStateCallback
#define RemoveNotifyCallback RemoveFileDescriptorStateCallback
#endif

TEST(Notify, Platform)
{
    using namespace std::chrono;
    NotifyManager             notify;
    PlatformFileHandleWrapper event1 = CreatePlatformEvent();
    PlatformFileHandleWrapper event2 = CreatePlatformEvent();

    steady_clock::time_point receive1;
    steady_clock::time_point receive2;
    steady_clock::time_point emit;
    size_t receiveCount1 = 0;
    size_t receiveCount2 = 0;
    auto   callbackId1   = notify.AddNotifyCallback(
        event1.NativeHandle(),
        [&receive1, &receiveCount1, &event1]()
        {
            receive1 = steady_clock::now();
            receiveCount1++;
            ResetPlatformEvent(event1);
        }
    );
    auto callbackId2 = notify.AddNotifyCallback(
        event2.NativeHandle(),
        [&receive2, &receiveCount2, &event2]()
        {
            receive2 = steady_clock::now();
            receiveCount2++;
            ResetPlatformEvent(event2);
        }
    );
    ASSERT_TRUE(callbackId1 != 0);
    ASSERT_TRUE(callbackId2 != 0);
    zeus::Sleep(seconds(1));

    PulsePlatformEvent(event1);
    emit = steady_clock::now();
    zeus::Sleep(seconds(1));
    if (emit > receive1)
    {
        auto diff = emit - receive1;
        EXPECT_LE(diff, kDeviation);
    }
    else
    {
        auto diff = receive1 - emit;
        EXPECT_LE(diff, kDeviation);
    }
    EXPECT_EQ(1, receiveCount1);
    EXPECT_EQ(0, receiveCount2);

    PulsePlatformEvent(event2);
    emit = steady_clock::now();
    zeus::Sleep(seconds(1));
    if (emit > receive2)
    {
        auto diff = emit - receive2;
        EXPECT_LE(diff, kDeviation);
    }
    else
    {
        auto diff = receive2 - emit;
        EXPECT_LE(diff, kDeviation);
    }
    EXPECT_EQ(1, receiveCount1);
    EXPECT_EQ(1, receiveCount2);

    PulsePlatformEvent(event1);
    emit = steady_clock::now();
    zeus::Sleep(seconds(1));
    if (emit > receive1)
    {
        auto diff = emit - receive1;
        EXPECT_LE(diff, kDeviation);
    }
    else
    {
        auto diff = receive1 - emit;
        EXPECT_LE(diff, kDeviation);
    }
    EXPECT_EQ(2, receiveCount1);
    EXPECT_EQ(1, receiveCount2);

    EXPECT_TRUE(notify.RemoveNotifyCallback(callbackId1));

    PulsePlatformEvent(event1);
    emit = steady_clock::now();
    zeus::Sleep(seconds(1));

    EXPECT_GE(emit - receive1, kDeviation);

    EXPECT_EQ(2, receiveCount1);
    EXPECT_EQ(1, receiveCount2);

    PulsePlatformEvent(event2);
    emit = steady_clock::now();
    zeus::Sleep(seconds(1));

    if (emit > receive2)
    {
        auto diff = emit - receive2;
        EXPECT_LE(diff, kDeviation);
    }
    else
    {
        auto diff = receive2 - emit;
        EXPECT_LE(diff, kDeviation);
    }

    EXPECT_EQ(2, receiveCount1);
    EXPECT_EQ(2, receiveCount2);

    PulsePlatformEvent(event1);
    emit = steady_clock::now();
    zeus::Sleep(seconds(1));

    EXPECT_GE(emit - receive1, kDeviation);

    EXPECT_EQ(2, receiveCount1);
    EXPECT_EQ(2, receiveCount2);

    EXPECT_TRUE(notify.RemoveNotifyCallback(callbackId2));
    PulsePlatformEvent(event2);
    emit = steady_clock::now();
    zeus::Sleep(seconds(1));

    EXPECT_GE(emit - receive2, kDeviation);

    EXPECT_EQ(2, receiveCount1);
    EXPECT_EQ(2, receiveCount2);
    emit = steady_clock::now();
    PulsePlatformEvent(event1);
    zeus::Sleep(seconds(1));

    EXPECT_GE(emit - receive1, kDeviation);

    EXPECT_EQ(2, receiveCount1);
    EXPECT_EQ(2, receiveCount2);
    size_t callbackId3 = 0;
    emit               = steady_clock::now();
    callbackId3        = notify.AddNotifyCallback(
        event2.NativeHandle(),
        [&notify, &receive2, &receiveCount2, &callbackId3]()
        {
            receive2 = steady_clock::now();
            receiveCount2++;
            notify.RemoveNotifyCallback(callbackId3, false);
        }
    );
    zeus::Sleep(seconds(1));
    if (emit > receive2)
    {
        auto diff = emit - receive2;
        EXPECT_LE(diff, kDeviation);
    }
    else
    {
        auto diff = receive2 - emit;
        EXPECT_LE(diff, kDeviation);
    }

    EXPECT_EQ(2, receiveCount1);
    EXPECT_EQ(3, receiveCount2);
}