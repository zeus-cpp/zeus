#pragma once

#include <memory>
#include <vector>
#include <type_traits>
#include "zeus/foundation/config/config.h"

namespace zeus
{
struct LayeredConfigImpl;
class LayeredConfig : public Config
{
public:
    LayeredConfig();
    ~LayeredConfig() override;
    LayeredConfig(const LayeredConfig&)            = delete;
    LayeredConfig& operator=(const LayeredConfig&) = delete;
    LayeredConfig(LayeredConfig&&)                 = delete;
    LayeredConfig& operator=(LayeredConfig&&)      = delete;

    //order越小优先级越高，writeable表示通过LayeredConfig的可写性，不代表被插入配置本身的可写性
    //label为空意味不能以名字来查找,空名的可以插入多个，非空label不允许重复，如果插入同名的配置后果自负
    //加锁需要上很大的锁，从性能角度和使用场景触发死锁考虑，AddConfiguration和RemoveConfiguration这种管理接口不保证线程安全。
    //必须在正式开始修改配置项前修完成对配置的管理或者确保修改操作和管理在同一线程中
    void      AddConfig(const ConfigPtr& config, const std::string& label, bool writeable, int32_t order = 0);
    void      RemoveConfig(const ConfigPtr& config);
    void      RemoveConfig(const std::string& label);
    ConfigPtr FindConfig(const std::string& label) const;
    template<typename C = Config, typename = typename std::enable_if<std::is_base_of<Config, C>::value>::type>
    std::shared_ptr<C>     FindTypeConfig(const std::string& label) const;
    std::vector<ConfigPtr> GetAllConfig() const;

    bool                                     HasConfigValue(const std::string& key) const override;
    zeus::expected<ConfigValue, ConfigError> GetConfigValue(const std::string& key) const override;
    zeus::expected<void, ConfigError>        SetConfigValue(const std::string& key, const ConfigValue& value) override;
    zeus::expected<void, ConfigError>        RemoveConfigValue(const std::string& key) override;
    std::vector<std::string>                 GetConfigKeys(const std::string& key) const override;
    //注意这里被管理的配置如果自身支持修改通知，那么哪怕不通过LayeredConfig修改此配置，LayeredConfig上的修改通知也会被触发。
    //如果被管理的配置自身不支持修改通知，那么只有通过LayeredConfig修改此配置，LayeredConfig上的通知才会被触发。
    size_t AddChangeNotify(const std::function<void(const ConfigPoint& key, const ConfigValue& value)>& notify) const override;
    size_t AddChangeNotify(const std::string& key, const std::function<void(const ConfigValue&)>& notify) const override;
    bool   RemoveChangeNotify(size_t id) const override;
private:
    std::unique_ptr<LayeredConfigImpl> _impl;
};
template<typename C, typename>
std::shared_ptr<C> LayeredConfig::FindTypeConfig(const std::string& label) const
{
    return std::dynamic_pointer_cast<C>(FindConfig(label));
}
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
