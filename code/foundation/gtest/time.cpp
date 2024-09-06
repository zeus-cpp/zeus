#include <gtest/gtest.h>
#include <list>
#include <zeus/foundation/time/absolute_timer.h>
#include <zeus/foundation/time/relative_timer.h>
#include <zeus/foundation/time/time.h>
#include <zeus/foundation/time/time_utils.h>
#include <zeus/foundation/thread/thread_pool.h>

using namespace std;
using namespace zeus;

const auto kDeviation = std::chrono::milliseconds(100);

static bool operator==(const std::tm& lhs, const std::tm& rhs)
{
    return lhs.tm_sec == rhs.tm_sec && lhs.tm_min == rhs.tm_min && lhs.tm_hour == rhs.tm_hour && lhs.tm_mday == rhs.tm_mday &&
           lhs.tm_mon == rhs.tm_mon && lhs.tm_year == rhs.tm_year && lhs.tm_wday == rhs.tm_wday && lhs.tm_yday == rhs.tm_yday &&
           lhs.tm_isdst == rhs.tm_isdst;
}

static void RecursionTask(RelativeTimer* timer)
{
    timer->AddDelayTimerTask(std::bind(RecursionTask, timer), std::chrono::milliseconds(1));
}

TEST(RelativeTimer, DeadLock)
{
    RelativeTimer relativeTimer;
    RecursionTask(&relativeTimer);
    Sleep(std::chrono::seconds(3));
    relativeTimer.Stop();
}

TEST(RelativeTimer, DelayTask)
{
    RelativeTimer relativeTimer;

    const auto kPeriod = std::chrono::milliseconds(500);

    bool       flag    = false;
    auto       preTime = std::chrono::steady_clock::now();
    const auto task    = [&]()
    {
        auto nowTime = std::chrono::steady_clock::now();
        EXPECT_GE(nowTime - preTime, kPeriod - kDeviation);
        EXPECT_LE(nowTime - preTime, kPeriod + kDeviation);
        flag = true;
    };

    flag       = false;
    preTime    = std::chrono::steady_clock::now();
    size_t id1 = relativeTimer.AddDelayTimerTask(task, kPeriod);
    ASSERT_NE(id1, 0);
    EXPECT_FALSE(flag);
    Sleep(kPeriod * 2);
    EXPECT_TRUE(flag);
    EXPECT_FALSE(relativeTimer.RemoveTimerTask(id1));

    flag       = false;
    preTime    = std::chrono::steady_clock::now();
    size_t id2 = relativeTimer.AddDelayTimerTask(task, kPeriod);
    ASSERT_NE(id2, 0);
    Sleep(kPeriod + kDeviation);
    EXPECT_TRUE(flag);
    EXPECT_FALSE(relativeTimer.RemoveTimerTask(id2));
    EXPECT_NE(id1, id2);

    flag       = false;
    preTime    = std::chrono::steady_clock::now();
    size_t id3 = relativeTimer.AddDelayTimerTask(task, kPeriod);
    ASSERT_NE(id3, 0);
    Sleep(kDeviation);
    EXPECT_TRUE(relativeTimer.RemoveTimerTask(id3));
    Sleep(kPeriod * 2 + kDeviation);
    EXPECT_FALSE(flag);
    EXPECT_NE(id1, id3);

    flag       = false;
    preTime    = std::chrono::steady_clock::now();
    size_t id4 = relativeTimer.AddDelayTimerTask(task, kDeviation);
    ASSERT_NE(id4, 0);
    relativeTimer.UpdateTimerTaskPeriod(id4, kPeriod);
    Sleep(kDeviation);
    EXPECT_FALSE(flag);
    Sleep(kPeriod);
    EXPECT_TRUE(flag);
    EXPECT_NE(id3, id4);

    flag       = false;
    preTime    = std::chrono::steady_clock::now();
    size_t id5 = relativeTimer.AddDelayTimerTask(task, kPeriod + kPeriod);
    ASSERT_NE(id5, 0);
    relativeTimer.UpdateTimerTaskPeriod(id5, kPeriod);
    Sleep(kPeriod + kDeviation);
    EXPECT_TRUE(flag);
    EXPECT_NE(id4, id5);
}

TEST(RelativeTimer, PeriodTask)
{
    RelativeTimer                                    relativeTimer;
    const auto                                       kPeriod     = std::chrono::milliseconds(1000);
    const size_t                                     repeatCount = 10;
    std::list<std::chrono::steady_clock::time_point> ticks;

    ticks.clear();
    ticks.emplace_back(std::chrono::steady_clock::now());
    auto id1 = relativeTimer.AddPeriodTimerTask(
        [&ticks](size_t /*count*/)
        {
            ticks.emplace_back(std::chrono::steady_clock::now());
            return true;
        },
        kPeriod
    );
    ASSERT_NE(id1, 0);
    Sleep(kPeriod * repeatCount + kDeviation);
    EXPECT_TRUE(relativeTimer.RemoveTimerTask(id1));
    EXPECT_EQ(1 + repeatCount, ticks.size());
    Sleep(kPeriod);
    EXPECT_EQ(1 + repeatCount, ticks.size());
    auto pre = ticks.front();
    ticks.pop_front();
    while (!ticks.empty())
    {
        auto back = ticks.front();
        EXPECT_GE(back - pre, kPeriod - kDeviation);
        EXPECT_LE(back - pre, kPeriod + kDeviation);
        ticks.pop_front();
        pre = back;
    }

    ticks.clear();
    ticks.emplace_back(std::chrono::steady_clock::now());
    auto id2 = relativeTimer.AddPeriodTimerTask(
        [&ticks](size_t /*count*/)
        {
            ticks.emplace_back(std::chrono::steady_clock::now());
            return true;
        },
        kPeriod * 2
    );
    ASSERT_NE(id2, 0);
    EXPECT_TRUE(relativeTimer.UpdateTimerTaskPeriod(id2, kPeriod));
    Sleep(kPeriod * repeatCount + kDeviation);
    EXPECT_TRUE(relativeTimer.RemoveTimerTask(id2));
    EXPECT_EQ(1 + repeatCount, ticks.size());
    Sleep(kPeriod);
    EXPECT_EQ(1 + repeatCount, ticks.size());
    pre = ticks.front();
    ticks.pop_front();
    while (!ticks.empty())
    {
        auto back = ticks.front();
        EXPECT_GE(back - pre, kPeriod - kDeviation);
        EXPECT_LE(back - pre, kPeriod + kDeviation);
        ticks.pop_front();
        pre = back;
    }

    ticks.clear();
    ticks.emplace_back(std::chrono::steady_clock::now());
    auto id3 = relativeTimer.AddPeriodTimerTask(
        [&ticks, repeatCount](size_t count)
        {
            ticks.emplace_back(std::chrono::steady_clock::now());
            return count < repeatCount / 2;
        },
        kPeriod
    );
    ASSERT_NE(id3, 0);
    Sleep(kPeriod * repeatCount + kDeviation);
    EXPECT_FALSE(relativeTimer.RemoveTimerTask(id3));
    EXPECT_EQ(1 + repeatCount / 2, ticks.size());
    Sleep(kPeriod);
    EXPECT_EQ(1 + repeatCount / 2, ticks.size());
    pre = ticks.front();
    ticks.pop_front();
    while (!ticks.empty())
    {
        auto back = ticks.front();
        EXPECT_GE(back - pre, kPeriod - kDeviation);
        EXPECT_LE(back - pre, kPeriod + kDeviation);
        ticks.pop_front();
        pre = back;
    }

    std::list<std::chrono::steady_clock::time_point> longTicks;
    bool                                             modify = false;
    ticks.clear();
    size_t id4 = 0;
    longTicks.emplace_back(std::chrono::steady_clock::now());
    id4 = relativeTimer.AddPeriodTimerTask(
        [&ticks, &longTicks, repeatCount, &modify, &relativeTimer, &id4, kPeriod](size_t /*count*/)
        {
            if (!modify)
            {
                longTicks.emplace_back(std::chrono::steady_clock::now());
                modify = true;
                EXPECT_TRUE(relativeTimer.UpdateTimerTaskPeriod(id4, kPeriod));
            }
            else
            {
                ticks.emplace_back(std::chrono::steady_clock::now());
            }
            return true;
        },
        kPeriod * 2
    );
    ASSERT_NE(id4, 0);
    Sleep(kPeriod * repeatCount + kDeviation);
    EXPECT_TRUE(relativeTimer.RemoveTimerTask(id4));
    EXPECT_EQ(2, longTicks.size());
    EXPECT_EQ(8, ticks.size());
    Sleep(kPeriod * 2);
    EXPECT_EQ(2, longTicks.size());
    EXPECT_EQ(8, ticks.size());

    pre = longTicks.front();
    longTicks.pop_front();
    while (!longTicks.empty())
    {
        auto back = longTicks.front();
        EXPECT_GE(back - pre, kPeriod * 2 - kDeviation);
        EXPECT_LE(back - pre, kPeriod * 2 + kDeviation);
        longTicks.pop_front();
        pre = back;
    }

    pre = ticks.front();
    ticks.pop_front();
    while (!ticks.empty())
    {
        auto back = ticks.front();
        EXPECT_GE(back - pre, kPeriod - kDeviation);
        EXPECT_LE(back - pre, kPeriod + kDeviation);
        ticks.pop_front();
        pre = back;
    }
}

TEST(AbsoluteTimer, base)
{
    AbsoluteTimer absoluteTimer;
    bool          flag             = false;
    const auto    kDuration        = std::chrono::seconds(6);
    const auto    kSecondDeviation = std::chrono::seconds(1);
    auto          preTime          = std::chrono::system_clock::now();
    auto          time             = std::chrono::system_clock::to_time_t(preTime + kDuration);
    tm            tagTime          = zeus::Localtime(time);
    absoluteTimer.AddAbsoluteTimerTask(

        [=, &flag]()
        {
            auto nowTime  = std::chrono::system_clock::now();
            auto duration = nowTime - preTime;
            auto debug    = std::chrono::duration_cast<std::chrono::microseconds>(duration);
            EXPECT_GE(nowTime - preTime, kDuration - kSecondDeviation);
            EXPECT_LE(nowTime - preTime, kDuration + kSecondDeviation);
            flag = true;
        },
        tagTime
    );
    Sleep(kDuration * 2);
    EXPECT_TRUE(flag);
    flag    = false;
    preTime = std::chrono::system_clock::now();
    time    = std::chrono::system_clock::to_time_t(preTime + kDuration);
    tagTime = zeus::Localtime(time);
    absoluteTimer.AddAbsoluteTimerTask(
        [=, &flag]()
        {
            auto nowTime = std::chrono::system_clock::now();
            EXPECT_GE(nowTime - preTime, kDuration - kSecondDeviation);
            EXPECT_LE(nowTime - preTime, kDuration + kSecondDeviation);
            flag = true;
        },
        tagTime
    );
    Sleep(kDuration * 2);
    EXPECT_TRUE(flag);
}

TEST(RelativeTimer, Concurrent)
{
    const size_t testCount = 1000;

    atomic_flag flag[testCount] = {ATOMIC_FLAG_INIT};

    ThreadPool pool(2, true, 4);

    auto setFlag = [&flag](size_t index)
    {
        ASSERT_EQ(false, flag[index].test_and_set());
    };

    auto filter = [&setFlag, &pool](size_t index)
    {
        if (index % 4 == 0)
        {
            for (int i = 0; i < 4; ++i)
            {
                pool.Commit(std::bind(setFlag, index + i));
            }
        }
    };

    RelativeTimer relativeTimer;
    for (size_t index = 0; index < testCount; ++index)
    {
        auto id = relativeTimer.AddDelayTimerTask(std::bind(filter, index), std::chrono::milliseconds(100));

        if (index && index % 5 == 0)
        {
            relativeTimer.RemoveTimerTask(id);
        }
    }
    Sleep(std::chrono::seconds(5));
    for (size_t index = 0; index < testCount; ++index)
    {
        if (index % 5 == 0)
        {
            index += 4;
        }
        EXPECT_TRUE(flag[index].test_and_set());
    }
}

TEST(RelativeTimer, Manual)
{
    auto relativeTimer = std::make_unique<RelativeTimer>(false);

    const auto kPeriod = std::chrono::milliseconds(1000);

    bool       flag    = false;
    auto       preTime = std::chrono::steady_clock::now();
    const auto task    = [&]()
    {
        auto nowTime = std::chrono::steady_clock::now();
        EXPECT_GE(nowTime - preTime, kPeriod * 2 - kDeviation * 2);
        EXPECT_LE(nowTime - preTime, kPeriod * 2 + kDeviation * 2);
        flag = true;
    };

    flag       = false;
    preTime    = std::chrono::steady_clock::now();
    size_t id1 = relativeTimer->AddSimplePeriodTimerTask(task, kPeriod);
    ASSERT_NE(id1, 0);
    EXPECT_FALSE(flag);
    Sleep(kPeriod * 2);
    EXPECT_FALSE(flag);
    relativeTimer->Start();
    Sleep(kPeriod);
    EXPECT_TRUE(flag);
    relativeTimer->Stop();
    EXPECT_FALSE(relativeTimer->RemoveTimerTask(id1));
    relativeTimer.reset();
    EXPECT_TRUE(true);
}

TEST(Time, SleepSecond)
{
    const auto kDuration = std::chrono::seconds(3);
    auto       preTime   = std::chrono::steady_clock::now();
    Sleep(kDuration);
    auto nowTime = std::chrono::steady_clock::now();
    EXPECT_GE(nowTime - preTime, kDuration - kDeviation);
    EXPECT_LE(nowTime - preTime, kDuration + kDeviation);
}

TEST(Time, SleepMillion)
{
    const auto kDuration = std::chrono::milliseconds(100);
    auto       preTime   = std::chrono::steady_clock::now();
    Sleep(kDuration);
    auto nowTime = std::chrono::steady_clock::now();
    EXPECT_GE(nowTime - preTime, kDuration - kDeviation);
    EXPECT_LE(nowTime - preTime, kDuration + kDeviation);
}

TEST(Time, Localtime)
{
    std::tm expectTm {};             // get_time does not set all fields hence {}
    expectTm.tm_year  = 2020 - 1900; // 2020
    expectTm.tm_mon   = 7 - 1;       // July
    expectTm.tm_mday  = 15;          // 15th
    expectTm.tm_hour  = 10;
    expectTm.tm_min   = 15;
    expectTm.tm_isdst = 0;
    std::time_t t     = std::mktime(&expectTm);
    {
        auto tm = zeus::Localtime(t);
        EXPECT_EQ(tm, expectTm);
    }
    {
        std::tm tm;
        ASSERT_TRUE(zeus::Localtime(t, tm));
        EXPECT_EQ(tm, expectTm);
    }
}

TEST(Time, Gmtime)
{
    std::tm expectTm {};             // get_time does not set all fields hence {}
    expectTm.tm_year  = 2020 - 1900; // 2020
    expectTm.tm_mon   = 7 - 1;       // July
    expectTm.tm_mday  = 15;          // 15th
    expectTm.tm_hour  = 10;
    expectTm.tm_min   = 15;
    expectTm.tm_isdst = 0;
    std::time_t t     = std::mktime(&expectTm);
    t += zeus::GetTimeZone();
    {
        auto tm = zeus::Gmtime(t);
        EXPECT_EQ(tm, expectTm);
    }
    {
        std::tm tm;
        ASSERT_TRUE(zeus::Gmtime(t, tm));
        EXPECT_EQ(tm, expectTm);
    }
}

TEST(Time, FormatSystemClock)
{
    auto now  = std::chrono::system_clock::now();
    auto tm   = zeus::Localtime(std::chrono::system_clock::to_time_t(now));
    auto year = FormatSystemClockTime(now, "%Y");
    EXPECT_EQ(tm.tm_year + 1900, std::stoul(year, nullptr, 10));

    auto month = FormatSystemClockTime(now, "%m");
    EXPECT_EQ(tm.tm_mon + 1, std::stoul(month, nullptr, 10));

    auto day = FormatSystemClockTime(now, "%d");
    EXPECT_EQ(tm.tm_mday, std::stoul(day, nullptr, 10));

    auto hour = FormatSystemClockTime(now, "%H");
    EXPECT_EQ(tm.tm_hour, std::stoul(hour, nullptr, 10));

    auto minute = FormatSystemClockTime(now, "%M");
    EXPECT_EQ(tm.tm_min, std::stoul(minute, nullptr, 10));

    auto second = FormatSystemClockTime(now, "%S");
    EXPECT_EQ(tm.tm_sec, std::stoul(second, nullptr, 10));
}

TEST(Time, FormatTimeT)
{
    std::time_t now  = std::time(nullptr);
    auto        tm   = zeus::Localtime(now);
    auto        year = FormatTime(now, "%Y");
    EXPECT_EQ(tm.tm_year + 1900, std::stoul(year));

    auto month = FormatTime(now, "%m");
    EXPECT_EQ(tm.tm_mon + 1, std::stoul(month));

    auto day = FormatTime(now, "%d");
    EXPECT_EQ(tm.tm_mday, std::stoul(day));

    auto hour = FormatTime(now, "%H");
    EXPECT_EQ(tm.tm_hour, std::stoul(hour));

    auto minute = FormatTime(now, "%M");
    EXPECT_EQ(tm.tm_min, std::stoul(minute));

    auto second = FormatTime(now, "%S");
    EXPECT_EQ(tm.tm_sec, std::stoul(second));
}

TEST(Time, FormatDuration)
{
    auto time     = std::chrono::seconds(60 * 60 * 24 * 105 + 60 * 60 * 4 + 60 * 56 + 34) + std::chrono::milliseconds(234);
    auto duration = FormatDuration(time);
    EXPECT_EQ(duration.day, 105);
    EXPECT_EQ(duration.hour, 4);
    EXPECT_EQ(duration.minute, 56);
    EXPECT_EQ(duration.second, 34);
    EXPECT_EQ(duration.millisecond, 234);
    EXPECT_EQ("105-04:56:34.234", FormatDurationStr(time));

    time = std::chrono::seconds(60 * 60 * 24 * 105 + 60 * 60 * 4 + 60 * 8 + 34) + std::chrono::milliseconds(2034);

    duration = FormatDuration(time);
    EXPECT_EQ(duration.day, 105);
    EXPECT_EQ(duration.hour, 4);
    EXPECT_EQ(duration.minute, 8);
    EXPECT_EQ(duration.second, 34 + 2);
    EXPECT_EQ(duration.millisecond, 34);
    EXPECT_EQ("105-04:08:36.034", FormatDurationStr(time));
}
