#include "zeus/foundation/dep_inject/managed_object.h"
#include "zeus/foundation/dep_inject/object_container.h"

namespace zeus
{
struct ManagedObjectImpl
{
};
ManagedObject::ManagedObject() : _impl(std::make_unique<ManagedObjectImpl>())
{
}
ManagedObject::~ManagedObject()
{
}

}
