#ifdef __linux__
#include <vector>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sys/types.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <gtest/gtest.h>
#include <zeus/foundation/string/string_utils.h>
#include <zeus/foundation/file/file_utils.h>
#include <zeus/foundation/system/net_adapter.h>
#include <zeus/foundation/byte/byte_utils.h>

namespace fs = std::filesystem;
using namespace zeus;

TEST(NetAdapter, ListAll)
{
    auto     adapters = NetAdapter::ListAll();
    ifaddrs* ifaddr   = nullptr;
    if (!getifaddrs(&ifaddr))
    {
        for (struct ifaddrs* ptr = ifaddr; ptr != nullptr; ptr = ptr->ifa_next)
        {
            if (ptr->ifa_addr != nullptr && (ptr->ifa_addr->sa_family == AF_INET || ptr->ifa_addr->sa_family == AF_INET6))
            {
                std::string name        = ptr->ifa_name;
                bool        isConnected = ptr->ifa_flags & IFF_RUNNING;
                bool        isLoopback  = ptr->ifa_flags & IFF_LOOPBACK;
                EXPECT_TRUE(std::any_of(
                    adapters.begin(), adapters.end(),
                    [name, isConnected, isLoopback](const NetAdapter& adapter)
                    {
                        EXPECT_EQ(adapter.GetId(), adapter.GetDeviceName());
                        EXPECT_EQ(adapter.GetId(), adapter.GetDisplayName());
                        if (adapter.GetId() == name)
                        {
                            EXPECT_EQ(isLoopback, NetAdapter::NetAdapterType::Loopback == adapter.GetType());
                            EXPECT_EQ(isConnected, adapter.IsConnected());
                            EXPECT_EQ(ToUpperCopy(Trim(FileContent(fs::path("/sys/class/net") / name / "address").value())), adapter.GetMac(":"));
                            return true;
                        }
                        return false;
                    }
                ));
            }
        }
        freeifaddrs(ifaddr);
    }
}

#endif
