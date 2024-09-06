#ifdef _WIN32
#include <map>
#include <gtest/gtest.h>
#include <netcon.h>
#include <zeus/foundation/string/string_utils.h>
#include <zeus/foundation/string/charset_utils.h>
#include <zeus/foundation/system/net_adapter.h>
#include <zeus/foundation/time/time.h>
#include <zeus/foundation/time/time_utils.h>
#include <zeus/foundation/resource/win/com_init.h>
#include <zeus/foundation/system/win/wmi.h>

using namespace zeus;

//TEST(NetChangeNotify, base)
//{
//    ComInit   intt(ComInit::ComType::APARTMENTTHREADED);
//    NetChangeNotify notify;
//    auto            id = notify.AddConnectChangeCallback([](const NetAdapter& /*network*/, NetChangeNotify::ConnectState /*state*/)
//                                              { std::cout << "NetChangeNotify" << std::endl; });
//    notify.Start();
//    Sleep(std::chrono::seconds(3));
//    EXPECT_TRUE(notify.RemoveConnectChangeCallback(id));
//    notify.Stop();
//}

TEST(NetAdapter, ListAll)
{
    auto                              nets = NetAdapter::ListAll();
    std::map<std::string, NetAdapter> netMap;
    for (const auto& net : nets)
    {
        netMap.emplace(net.GetId(), net);
    }
    auto query = WinWMI::Create();
    ASSERT_TRUE(query.has_value());
    auto                                  netConfigs = query->Query("SELECT * FROM Win32_NetworkAdapterConfiguration").value();
    std::map<std::string, nlohmann::json> netConfigMap;
    for (auto& net : netConfigs)
    {
        netConfigMap[net["SettingID"].get<std::string>()] = net;
    }
    auto wmiQuery = WinWMI::Create(R"(ROOT\WMI)");
    ASSERT_TRUE(wmiQuery.has_value());
    auto                       netTypes = wmiQuery->Query("SELECT * FROM MSNdis_PhysicalMediumType").value();
    std::map<std::string, int> netTypeMap;
    for (auto& net : netTypes)
    {
        netTypeMap[net["InstanceName"].get<std::string>()] = net["NdisPhysicalMediumType"].get<int>();
    }
    auto netIterfaces = query->Query("SELECT * FROM Win32_NetworkAdapter").value();
    for (auto& net : netIterfaces)
    {
        if (net["GUID"].is_null())
        {
            continue;
        }
        auto id = net["GUID"].get<std::string>();
        if (!id.empty())
        {
            int  status = net["NetConnectionStatus"].get<int>();
            bool enable =
                status == NCS_CONNECTED || status == NCS_MEDIA_DISCONNECTED || status == NCS_AUTHENTICATING || status == NCS_AUTHENTICATION_SUCCEEDED;

            auto adapterIter = netMap.find(id);
            ASSERT_TRUE(adapterIter != netMap.end());
            EXPECT_TRUE(adapterIter->second.GetDisplayName() == net["NetConnectionID"].get<std::string>());
            EXPECT_EQ(enable, adapterIter->second.IsEnable());
            if (enable)
            {
                EXPECT_TRUE(adapterIter->second.GetDeviceName() == net["Name"].get<std::string>());
                EXPECT_TRUE(adapterIter->second.GetMac() == net["MACAddress"].get<std::string>());
                EXPECT_EQ(adapterIter->second.IsConnected(), net["NetEnabled"].get<bool>());
                if (adapterIter->second.IsConnected())
                {
                    if (!adapterIter->second.GetIPV4Address().empty() || !net["Speed"].is_null())
                    {
                        EXPECT_EQ(std::to_string(adapterIter->second.GetSpeed()), net["Speed"].get<std::string>());
                    }
                    std::vector<std::string> expectV4Ips;
                    std::vector<std::string> expectV6Ips;
                    std::vector<std::string> expectV4Masks;
                    std::vector<std::string> expectV6Prefixes;
                    if (netConfigMap[id].contains("IPAddress"))
                    {
                        auto& ipAddress = netConfigMap[id]["IPAddress"];
                        for (const auto& address : ipAddress)
                        {
                            auto ip = address.get<std::string>();
                            if (ip.find('.') != std::string::npos)
                            {
                                expectV4Ips.emplace_back(ip);
                            }
                            if (ip.find(':') != std::string::npos)
                            {
                                expectV6Ips.emplace_back(ip);
                            }
                        }
                    }
                    if (netConfigMap[id].contains("IPSubnet"))
                    {
                        auto& ipSubnets = netConfigMap[id]["IPSubnet"];
                        for (const auto& ipSubnet : ipSubnets)
                        {
                            auto subnet = ipSubnet.get<std::string>();
                            if (subnet.find('.') != std::string::npos)
                            {
                                expectV4Masks.emplace_back(subnet);
                            }
                            else
                            {
                                expectV6Prefixes.emplace_back(subnet);
                            }
                        }
                    }
                    auto ipv4s = adapterIter->second.GetIPV4Address();
                    for (auto iter = ipv4s.begin(); iter != ipv4s.end(); iter++)
                    {
                        if (iter->Address() == "0.0.0.0")
                        {
                            ipv4s.erase(iter);
                            break;
                        }
                    }
                    ASSERT_EQ(ipv4s.size(), expectV4Ips.size());
                    for (auto i = 0U; i < ipv4s.size(); i++)
                    {
                        EXPECT_EQ(ipv4s[i].Address(), expectV4Ips[i]);
                        EXPECT_EQ(ipv4s[i].Mask(), expectV4Masks[i]);
                    }
                    auto ipv6s = adapterIter->second.GetIPV6Address();
                    ASSERT_EQ(ipv6s.size(), expectV6Ips.size());
                    for (auto i = 0U; i < ipv6s.size(); i++)
                    {
                        EXPECT_EQ(ipv6s[i].Address(), expectV6Ips[i]);
                        EXPECT_EQ(std::to_string(ipv6s[i].PrefixLength()), expectV6Prefixes[i]);
                    }
                    std::vector<std::string> expectGateways;
                    if (netConfigMap[id].contains("DefaultIPGateway"))
                    {
                        auto& gateways = netConfigMap[id]["DefaultIPGateway"];
                        for (const auto& gatewayAddress : gateways)
                        {
                            auto gateway = gatewayAddress.get<std::string>();
                            if (gateway.find('.') != std::string::npos)
                            {
                                expectGateways.emplace_back(gateway);
                            }
                        }
                    }
                    ASSERT_EQ(adapterIter->second.GetIPV4Gateway().size(), expectGateways.size());
                    for (auto& gateway : adapterIter->second.GetIPV4Gateway())
                    {
                        EXPECT_NE(std::find(expectGateways.begin(), expectGateways.end(), gateway), expectGateways.end());
                    }
                }
            }
            EXPECT_TRUE(
                NetAdapter::NetAdapterType::Ethernet == adapterIter->second.GetType() == (netTypeMap[adapterIter->second.GetDeviceName()] == 0)
            );
            EXPECT_TRUE(
                NetAdapter::NetAdapterType::Wireless == adapterIter->second.GetType() == (netTypeMap[adapterIter->second.GetDeviceName()] == 9)
            );
            EXPECT_EQ(
                adapterIter->second.IsVirtual(),
                StartWith(net["PNPDeviceID"].get<std::string>(), "ROOT") || StartWith(net["PNPDeviceID"].get<std::string>(), "COMPOSITEBUS")
            );

            EXPECT_EQ(enable, adapterIter->second.IsEnable());
        }
    }
}

#endif