#include "zeus/foundation/sync/condition_variable.h"
namespace zeus
{
void ConditionVariable::lock()
{
    return Lock();
}
void ConditionVariable::unlock()
{
    return Unlock();
}
bool ConditionVariable::try_lock()
{
    return TryLock();
}
}
