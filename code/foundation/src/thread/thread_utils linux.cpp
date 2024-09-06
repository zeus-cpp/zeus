#include "zeus/foundation/thread/thread_utils.h"
#ifdef __linux__
#include <unistd.h>
#include <sys/prctl.h>
namespace zeus
{
// 只能显示16个字符，包含了'\0'
void SetThreadName(const std::string& threadName)
{
    prctl(PR_SET_NAME, threadName.c_str());
}

uint64_t GetThreadId()
{
    return gettid();
}

} // namespace zeus
#endif