#include "zeus/foundation/dep_inject/auto_type_register.h"
#include <cassert>
#include "zeus/foundation/container/callback_manager.hpp"
namespace zeus
{
CallbackManager<ObjectConstainer&>& GetExecutorManager()
{
    static CallbackManager<ObjectConstainer&> executorManager;
    return executorManager;
}

size_t AutoTypeRegister::AddTypeRegisterTask(const RegisterTask& task)
{
    assert(task);
    return GetExecutorManager().AddCallback(task);
}
void AutoTypeRegister::ExecuteTypeRegisterTask(ObjectConstainer& container, bool clear)
{
    GetExecutorManager().Call(container);
    if (clear)
    {
        GetExecutorManager().Clear();
    }
}
} // namespace zeus