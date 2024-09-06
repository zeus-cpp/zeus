#pragma once
#include <memory>
namespace zeus
{
struct ManagedObjectImpl;
//此类主要为了建立对象容器管理的统一抽象，以及对被管理对象的限制
//继承了此类的类应该都是单例的
class ManagedObject
{
public:
    ManagedObject();
    virtual ~ManagedObject();
    ManagedObject(const ManagedObject&)            = delete;
    ManagedObject(ManagedObject&&)                 = delete;
    ManagedObject& operator=(const ManagedObject&) = delete;
private:
    std::unique_ptr<ManagedObjectImpl> _impl;
};

} // namespace zeus
#include "zeus/foundation/core/zeus_compatible.h"
