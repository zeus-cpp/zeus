#include "zeus/foundation/dep_inject/object_container.h"
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <algorithm>
#include "zeus/foundation/dep_inject/injected_object.h"

namespace zeus
{
struct MetaClass
{
    std::type_index                   mainType = typeid(void);
    std::list<std::type_index>        baseTypes;
    ObjectConstainer::ConstructorType constructor;
    std::shared_ptr<ManagedObject>    instance;
    size_t                            referenceCount = 0;
    std::string                       customName;
};
struct ObjectConstainerImpl
{
    std::unordered_multimap<std::type_index, std::shared_ptr<MetaClass>> metaClasses;
    std::unordered_map<std::string, std::shared_ptr<MetaClass>>          customNameClasses;
};

std::shared_ptr<ManagedObject> CreateAutoReference(MetaClass& metaClass, ObjectConstainer& container)
{
    if (!metaClass.instance)
    {
        assert(metaClass.constructor);
        metaClass.instance = metaClass.constructor();
        assert(metaClass.instance);
        auto injectObject = std::dynamic_pointer_cast<InjectedObject>(metaClass.instance);
        if (injectObject)
        {
            injectObject->Inject(container);
        }
    }
    ++metaClass.referenceCount;
    return std::shared_ptr<ManagedObject>(
        metaClass.instance.get(),
        [&metaClass](ManagedObject* object)
        {
            assert(metaClass.instance.get() == object);
            --metaClass.referenceCount;
            if (metaClass.referenceCount == 0)
            {
                auto injectObject = std::dynamic_pointer_cast<InjectedObject>(metaClass.instance);
                if (injectObject)
                {
                    injectObject->Uninject();
                }
                metaClass.instance.reset();
            }
        }
    );
}

ObjectConstainer::ObjectConstainer() : _impl(std::make_unique<ObjectConstainerImpl>())
{
}
ObjectConstainer::~ObjectConstainer()
{
}

bool ObjectConstainer::RegisterType(
    const std::type_index& mainType, std::list<std::type_index>&& baseTypes, const ConstructorType& constructor,
    const std::shared_ptr<ManagedObject>& instance, const std::string& customClassName
)
{
    if (!customClassName.empty() && _impl->customNameClasses.find(customClassName) != _impl->customNameClasses.end())
    {
        assert(false);
        //不允许相同的自定义类型名
        return false;
    }
    assert(constructor || instance);
    auto metaClass         = std::make_shared<MetaClass>();
    metaClass->mainType    = mainType;
    metaClass->baseTypes   = std::move(baseTypes);
    metaClass->constructor = constructor;
    metaClass->instance    = instance;
    metaClass->customName  = customClassName;
    if (metaClass->instance)
    {
        auto injectObject = std::dynamic_pointer_cast<InjectedObject>(metaClass->instance);
        if (injectObject)
        {
            injectObject->Inject(*this);
        }
        metaClass->referenceCount = 1;
    }
    _impl->metaClasses.emplace(metaClass->mainType, metaClass); //插入主类型索引
    for (auto& type : metaClass->baseTypes)                     //插入基类型索引
    {
        _impl->metaClasses.emplace(type, metaClass);
    }
    if (!metaClass->customName.empty())
    {
        _impl->customNameClasses.emplace(metaClass->customName, metaClass);
    }
    return true;
}

std::shared_ptr<ManagedObject> ObjectConstainer::GetObjectReference(const std::type_index& type)
{
    if (auto iter = _impl->metaClasses.find(type); iter != _impl->metaClasses.end())
    {
        return CreateAutoReference(*iter->second, *this);
    }
    return nullptr;
}

std::shared_ptr<ManagedObject> ObjectConstainer::GetObjectReference(const std::type_index& type, const std::string& customClassName)
{
    if (auto iter = _impl->customNameClasses.find(customClassName); iter != _impl->customNameClasses.end())
    {
        auto& MetaClass = *iter->second;
        if (MetaClass.mainType == type ||
            std::any_of(
                MetaClass.baseTypes.begin(), MetaClass.baseTypes.end(), [&type](const std::type_index& baseType) { return baseType == type; }
            ))
        {
            return CreateAutoReference(*iter->second, *this);
        }
    }
    return nullptr;
}

} // namespace zeus
