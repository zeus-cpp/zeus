#include <gtest/gtest.h>
#include <zeus/foundation/time/time.h>
#include <zeus/foundation/system/net_adapter.h>
#include <zeus/foundation/system/net_adapter_notify.h>

using namespace zeus;

TEST(NetAdapter, List)
{
    auto all = NetAdapter::ListAll();
    {
        auto ethList      = NetAdapter::List(NetAdapter::NetAdapterType::Ethernet, NetAdapter::NetVersion::V4);
        auto wirelessList = NetAdapter::List(NetAdapter::NetAdapterType::Wireless, NetAdapter::NetVersion::V4);
        auto loopbackList = NetAdapter::List(NetAdapter::NetAdapterType::Loopback, NetAdapter::NetVersion::V4);
        for (const auto& adapter : ethList)
        {
            EXPECT_EQ(adapter.GetType(), NetAdapter::NetAdapterType::Ethernet);
            EXPECT_TRUE(adapter.GetIPV6Address().empty());
            EXPECT_TRUE(adapter.GetIPV6Gateway().empty());
        }
        for (const auto& adapter : wirelessList)
        {
            EXPECT_EQ(adapter.GetType(), NetAdapter::NetAdapterType::Wireless);
            EXPECT_TRUE(adapter.GetIPV6Address().empty());
            EXPECT_TRUE(adapter.GetIPV6Gateway().empty());
        }
        for (const auto& adapter : loopbackList)
        {
            EXPECT_EQ(adapter.GetType(), NetAdapter::NetAdapterType::Loopback);
            EXPECT_TRUE(adapter.GetIPV6Address().empty());
            EXPECT_TRUE(adapter.GetIPV6Gateway().empty());
        }
        EXPECT_EQ(all.size(), ethList.size() + wirelessList.size() + loopbackList.size());
    }
    {
        auto ethList      = NetAdapter::List(NetAdapter::NetAdapterType::Ethernet, NetAdapter::NetVersion::V6);
        auto wirelessList = NetAdapter::List(NetAdapter::NetAdapterType::Wireless, NetAdapter::NetVersion::V6);
        auto loopbackList = NetAdapter::List(NetAdapter::NetAdapterType::Loopback, NetAdapter::NetVersion::V6);

        EXPECT_EQ(all.size(), ethList.size() + wirelessList.size() + loopbackList.size());

        for (const auto& adapter : ethList)
        {
            EXPECT_EQ(adapter.GetType(), NetAdapter::NetAdapterType::Ethernet);
            EXPECT_TRUE(adapter.GetIPV4Address().empty());
            EXPECT_TRUE(adapter.GetIPV4Gateway().empty());
        }
        for (const auto& adapter : wirelessList)
        {
            EXPECT_EQ(adapter.GetType(), NetAdapter::NetAdapterType::Wireless);
            EXPECT_TRUE(adapter.GetIPV4Address().empty());
            EXPECT_TRUE(adapter.GetIPV4Gateway().empty());
        }
        for (const auto& adapter : loopbackList)
        {
            EXPECT_EQ(adapter.GetType(), NetAdapter::NetAdapterType::Loopback);
            EXPECT_TRUE(adapter.GetIPV4Address().empty());
            EXPECT_TRUE(adapter.GetIPV4Gateway().empty());
        }
    }
}

TEST(NetAdapter, Notify)
{
    NetAdapterNotify notify;
    auto             id = notify.AddConnectChangeCallback([](const std::string& adapterId, NetAdapterNotify::ConnectState state)
                                              { std::cout << adapterId << ":" << NetAdapterNotify::StateName(state) << std::endl; });
    notify.Start();
    Sleep(std::chrono::seconds(2));
    notify.Stop();
    notify.RemoveConnectChangeCallback(id);
}

TEST(NetAdapter, Find)
{
    EXPECT_FALSE(NetAdapter::FindNetAdapterById("xxx").has_value());
    auto nets = NetAdapter::ListAll();
    for (const auto& item : nets)
    {
        auto adapter = NetAdapter::FindNetAdapterById(item.GetId());
        EXPECT_TRUE(adapter.has_value());
        EXPECT_EQ(adapter.value().GetId(), item.GetId());
        EXPECT_EQ(adapter.value().GetDisplayName(), item.GetDisplayName());
        EXPECT_EQ(adapter.value().GetDeviceName(), item.GetDeviceName());
        EXPECT_EQ(adapter.value().GetMac(), item.GetMac());
        EXPECT_EQ(adapter.value().GetSpeed(), item.GetSpeed());
        EXPECT_EQ(adapter.value().GetType(), item.GetType());
        EXPECT_EQ(adapter.value().IsConnected(), item.IsConnected());
        EXPECT_EQ(adapter.value().IsVirtual(), item.IsVirtual());
    }
}