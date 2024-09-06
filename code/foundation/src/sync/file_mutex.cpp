#include "zeus/foundation/sync/file_mutex.h"
namespace zeus
{
void FileMutex::lock()
{
    LockExclusive();
}
void FileMutex::lock_shared()
{
    LockShared();
}
void FileMutex::unlock()
{
    Unlock();
}
bool FileMutex::try_lock()
{
    return TryLockExclusive().value_or(false);
}
bool FileMutex::try_lock_shared()
{
    return TryLockShared().value_or(false);
}
} // namespace zeus
