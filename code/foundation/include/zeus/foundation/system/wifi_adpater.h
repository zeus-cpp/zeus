#pragma once

#ifdef _WIN32
#include <memory>
#include <vector>
#include <functional>
#include <string>

namespace zeus
{
struct WifiBssImpl;
class WifiBss
{
public:
    enum class WifiStrength
    {
        Weak,
        Fair,
        Good,
        Excellent,
    };
    ~WifiBss();
    WifiBss(const WifiBss& other);
    WifiBss(WifiBss&& other);
    WifiBss&     operator=(const WifiBss& other);
    WifiBss&     operator=(WifiBss&& other) noexcept;
    std::string  Mac(const std::string& split = ":") const;
    std::string  SSID() const;
    //当无法获取到rssi时，会根据信号质量线性换算一个大概的rssi值
    int32_t      Rssi() const;
    WifiStrength Strength() const;
    bool         IsConnected() const;
    //信号质量取值范围0~100
    uint8_t      SignalQuality() const;
    size_t       Frequency();
public:
    //信号质量换算出的rssi和信号强度是线性拉伸的，并不精确，直接获取rssi或者从rssi换算的信号强度比较准确
    static int32_t      CastSignalQualityToRssi(uint8_t quality);
    static WifiStrength CastSignalQualityToStrength(uint8_t quality);
    static WifiStrength CastRssiToStrength(int32_t rssi);
    friend class WifiAp;
    friend class WifiAdpater;
private:
    WifiBss();
private:
    std::unique_ptr<WifiBssImpl> _impl;
};

struct WifiApImpl;
class WifiAp
{
public:
    ~WifiAp();
    WifiAp(const WifiAp& other);
    WifiAp(WifiAp&& other);
    WifiAp&              operator=(const WifiAp& other);
    WifiAp&              operator=(WifiAp&& other) noexcept;
    std::string          SSID() const;
    std::vector<WifiBss> BssList() const;
    uint8_t              MaxSignalQuality() const;
    friend class WifiAdpater;
private:
    WifiAp();
private:
    std::unique_ptr<WifiApImpl> _impl;
};

struct WifiAdpaterImpl;
class WifiAdpater
{
public:
    ~WifiAdpater();
    WifiAdpater(const WifiAdpater& other);
    WifiAdpater(WifiAdpater&& other);
    //windows在已经连接wifi网络后会停止继续扫描其他信号，会导致获取不到其他的ap，如果想要扫描其他信号请使用扫描接口
    std::vector<WifiAp>      ListAp() const;
    std::string              Id() const;
    std::string              Name() const;
    //获取当前已连接信息不受扫描影响
    std::shared_ptr<WifiBss> GetConnectedBss() const;
    bool                     IsConnected() const;
    //扫描接口立刻返回，但是完整的扫描需要最多4s才能完成，可以通过WifiChangeNotify的扫描通知来确定扫描完成。调用扫描接口会导致当前ap列表被清空，请不要调用扫描后立即调用listap接口
    bool                     Scan();
public:
    //windows wifi接口api和Wireless Zero Configuration服务相关，如果获取不到可以查看此服务是否正常
    static std::vector<WifiAdpater> List();
    friend class WifiChangeNotify;
private:
    WifiAdpater();
private:
    std::unique_ptr<WifiAdpaterImpl> _impl;
};

struct WifiChangeNotifyImpl;
class WifiChangeNotify
{
public:
    WifiChangeNotify();
    ~WifiChangeNotify();
    void AddSignChangeCallback(const std::function<void(const WifiAdpater& adpater, size_t signal)>& callback);
    void AddConnectCallback(const std::function<void(const WifiAdpater& adpater, const std::string& ssid, bool state)>& callback);
    void AddDeviceCallback(const std::function<void(const WifiAdpater& adpater, bool state)>& callback);
    void AddScanCallback(const std::function<void(const WifiAdpater& adpater)>& callback);
    //启动通常是不会失败的，如果失败请考虑操作系统版本问题或者操作系统组件残缺
    bool Start();
    void Stop();
private:
    std::unique_ptr<WifiChangeNotifyImpl> _impl;
};
} // namespace zeus

#endif

#include "zeus/foundation/core/zeus_compatible.h"
