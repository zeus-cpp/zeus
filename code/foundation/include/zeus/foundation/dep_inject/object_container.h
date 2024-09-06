#pragma once
#include <string>
#include <functional>
#include <type_traits>
#include <typeinfo>
#include <typeindex>
#include <list>
#include <memory>
#include <cassert>
#include "zeus/foundation/dep_inject/managed_object.h"

namespace zeus
{
struct ObjectConstainerImpl;
class ObjectConstainer
{
public:
    using ConstructorType = std::function<std::shared_ptr<ManagedObject>()>;

    ObjectConstainer();
    ~ObjectConstainer();
    ObjectConstainer(const ObjectConstainer&)            = delete;
    ObjectConstainer(ObjectConstainer&&)                 = delete;
    ObjectConstainer& operator=(const ObjectConstainer&) = delete;

    template<typename MainType, typename... BaseType>
    bool RegisterType(const std::string& customClassName = "")
    {
        static_assert(std::is_base_of<ManagedObject, MainType>::value, "MainType must be derived from ManagedObject");
        return RegisterType(
            typeid(MainType), {typeid(BaseType)...}, []() { return std::make_shared<MainType>(); }, nullptr, customClassName
        );
    }

    template<typename MainType, typename... BaseType>
    bool RegisterType(const std::function<std::shared_ptr<ManagedObject>()>& constructor, const std::string& customClassName = "")
    {
        static_assert(std::is_base_of<ManagedObject, MainType>::value, "MainType must be derived from ManagedObject");
        return RegisterType(typeid(MainType), {typeid(BaseType)...}, constructor, nullptr, customClassName);
    }

    template<typename MainType, typename... BaseType>
    bool AddObject(std::shared_ptr<MainType> object, const std::string& customClassName = "")
    {
        static_assert(std::is_base_of<ManagedObject, MainType>::value, "MainType must be derived from ManagedObject");
        return RegisterType(typeid(MainType), {typeid(BaseType)...}, nullptr, object, customClassName);
    }

    template<typename Type>
    std::shared_ptr<Type> GetObjectReference(const std::string& customClassName = "")
    {
        std::shared_ptr<ManagedObject> object;
        if (!customClassName.empty())
        {
            object = GetObjectReference(typeid(Type), customClassName);
        }
        else
        {
            object = GetObjectReference(typeid(Type));
        }

        if (object)
        {
            auto reference = std::dynamic_pointer_cast<Type>(object);
            //理论上不会为空，因为获取的时候做了类型检查，但是为了保险起见还是加上
            assert(reference);
            return reference;
        }
        return nullptr;
    }
private:
    bool RegisterType(
        const std::type_index& mainType, std::list<std::type_index>&& baseTypes, const ConstructorType& constructor,
        const std::shared_ptr<ManagedObject>& instance, const std::string& customClassName
    );

    std::shared_ptr<ManagedObject> GetObjectReference(const std::type_index& type);

    std::shared_ptr<ManagedObject> GetObjectReference(const std::type_index& type, const std::string& customClassName);

private:
    std::unique_ptr<ObjectConstainerImpl> _impl;
};
} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
