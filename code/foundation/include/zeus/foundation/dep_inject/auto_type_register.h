#pragma once
#include <memory>
#include <string>
#include <functional>
#include "zeus/foundation/dep_inject/object_container.h"
namespace zeus
{
class AutoTypeRegister
{
public:
    using RegisterTask = std::function<void(ObjectConstainer& container)>;
    static size_t AddTypeRegisterTask(const RegisterTask& task);
    static void   ExecuteTypeRegisterTask(ObjectConstainer& container, bool clear);
};

}
#include "zeus/foundation/core/zeus_compatible.h"
