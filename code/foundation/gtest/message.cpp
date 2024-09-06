#include <gtest/gtest.h>
#include <zeus/foundation/message/message_bus.h>
#include <zeus/foundation/sync/event.h>
using namespace zeus;

TEST(MessageBus, base)
{
    MessageBus        bus;
    const std::string topic1 = "topic1";
    const std::string topic2 = "topic2";
    const std::string topic3 = "topic3";
    zeus::Event       event1;
    zeus::Event       event2;
    zeus::Event       event3;
    bus.Subscribe<const std::string &>(
        topic1, std::function<void(const std::string &)>(
                    [&](const std::string &data1)
                    {
                        EXPECT_EQ(data1, topic1 + topic1);
                        event1.Notify();
                    }
                )
    );
    bus.Subscribe<std::string, const std::string &>(
        topic2, std::function<void(std::string data1, const std::string &data2)>(
                    [&](std::string data1, const std::string &data2)
                    {
                        EXPECT_EQ(data1, topic2 + topic2);
                        EXPECT_EQ(data2, topic2 + topic2 + topic2);
                        event2.Notify();
                    }
                )
    );
    bus.Subscribe<std::string, std::string, std::string>(
        topic3, std::function<void(std::string data1, std::string data2, std::string data3)>(
                    [&](std::string data1, std::string data2, std::string data3)
                    {
                        EXPECT_EQ(data1, topic3 + topic3);
                        EXPECT_EQ(data2, topic3 + topic3 + topic3);
                        EXPECT_EQ(data3, topic3 + topic3 + topic3 + topic3);
                        event3.Notify();
                    }
                )
    );
    EXPECT_EQ(1, bus.Publish<const std::string &>(topic1, topic1 + topic1));
    event1.Wait();
    bus.Publish<std::string, const std::string &>(topic2, topic2 + topic2, topic2 + topic2 + topic2);
    event2.Wait();
    bus.Publish<std::string, std::string, std::string>(topic3, topic3 + topic3, topic3 + topic3 + topic3, topic3 + topic3 + topic3 + topic3);
    event3.Wait();
}
