#pragma once
#include <string>
#include <list>
#include <functional>
#include "zeus/foundation/dep_inject/object_container.h"
#include "zeus/foundation/dep_inject/managed_object.h"
#include "zeus/foundation/resource/auto_constructor.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage,cert-dcl37-c,cert-dcl51-cpp,bugprone-macro-parentheses) 没办法，要玩魔法，只能用宏
#define INJECT_TYPE_OBJECT(Type, valName)                                                                                 \
    std::shared_ptr<Type>       valName = nullptr;                                                                        \
    const zeus::AutoConstructor _INJECT_##valName {[this]()                                                               \
                                                   {                                                                      \
                                                       auto injectionFunction = [this](zeus::ObjectConstainer& container) \
                                                       {                                                                  \
                                                           if (valName)                                                   \
                                                           {                                                              \
                                                               return true;                                               \
                                                           }                                                              \
                                                           valName = container.GetObjectReference<Type>();                \
                                                           return valName != nullptr;                                     \
                                                       };                                                                 \
                                                       AddInjectTask(std::move(injectionFunction));                       \
                                                   }};                                                                    \
    const zeus::AutoConstructor _UNINJECT_##valName {[this]()                                                             \
                                                     {                                                                    \
                                                         auto uninjectionFunction = [this]()                              \
                                                         {                                                                \
                                                             valName.reset();                                             \
                                                         };                                                               \
                                                         AddUninjectTask(std::move(uninjectionFunction));                 \
                                                     }};

#define INJECT_NAME_OBJECT(Type, name, valName)                                                                             \
    std::shared_ptr<Type>       valName           = nullptr;                                                                \
    const zeus::AutoConstructor _INJECT_##valName = {[this]()                                                               \
                                                     {                                                                      \
                                                         auto injectionFunction = [this](zeus::ObjectConstainer& container) \
                                                         {                                                                  \
                                                             if (valName)                                                   \
                                                             {                                                              \
                                                                 return true;                                               \
                                                             }                                                              \
                                                             valName = container.GetObjectReference<Type>(#name);           \
                                                             return valName != nullptr;                                     \
                                                         };                                                                 \
                                                         AddInjectTask(std::move(injectionFunction));                       \
                                                     }};                                                                    \
    const zeus::AutoConstructor _UNINJECT_##valName {[this]()                                                               \
                                                     {                                                                      \
                                                         auto uninjectionFunction = [this]()                                \
                                                         {                                                                  \
                                                             valName.reset();                                               \
                                                         };                                                                 \
                                                         AddUninjectTask(std::move(uninjectionFunction));                   \
                                                     }};

// NOLINTEND(cppcoreguidelines-macro-usage,cert-dcl37-c,cert-dcl51-cpp,bugprone-macro-parentheses)
namespace zeus
{
struct InjectedObjectImpl;
class InjectedObject : virtual public ManagedObject
{
public:
    InjectedObject();
    ~InjectedObject() override;
    InjectedObject(const InjectedObject&)            = delete;
    InjectedObject(InjectedObject&&)                 = delete;
    InjectedObject& operator=(const InjectedObject&) = delete;
    bool            Inject(ObjectConstainer& container);
    void            Uninject();
protected:
    void AddInjectTask(std::function<bool(ObjectConstainer&)>&& task);
    void AddUninjectTask(std::function<void()>&& task);
protected:
    std::unique_ptr<InjectedObjectImpl> _impl;
};

} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
