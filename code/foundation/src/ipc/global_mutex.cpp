#include "zeus/foundation/ipc/global_mutex.h"
namespace zeus
{
void GlobalMutex::lock()
{
    Lock();
}
void GlobalMutex::unlock()
{
    Unlock();
}
bool GlobalMutex::try_lock()
{
    return TryLock().value_or(false);
}
} // namespace zeus
