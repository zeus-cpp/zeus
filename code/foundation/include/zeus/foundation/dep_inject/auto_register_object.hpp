#pragma once
#include <string>
#include <list>
#include <functional>
#include "zeus/foundation/dep_inject/object_container.h"
#include "zeus/foundation/dep_inject/auto_type_register.h"
#include "zeus/foundation/resource/auto_constructor.h"

namespace zeus
{
template<typename MainType, typename... BaseType>
class AutoRegisteObject : virtual public ManagedObject
{
public:
    AutoRegisteObject()
    {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
#endif
        _autoRegister; //这里只是为了防止编译器把静态变量优化掉，其实啥也不干
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
    }
    virtual ~AutoRegisteObject() {}
private:
    static void RegisterTask()
    {
        AutoTypeRegister::RegisterTask registerTypeTask = [](ObjectConstainer &container)
        {
            ObjectConstainer::ConstructorType constructor = []()
            {
                return std::make_shared<MainType>();
            };
            container.RegisterType<MainType, BaseType...>(constructor);
        };
        AutoTypeRegister::AddTypeRegisterTask(registerTypeTask);
    }
public:
    static const AutoConstructor _autoRegister;
};

template<typename MainType, typename... BaseType>
const AutoConstructor AutoRegisteObject<MainType, BaseType...>::_autoRegister(RegisterTask);

} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
