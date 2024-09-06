#include "zeus/foundation/time/time_utils.h"
#ifdef _WIN32
#include <Windows.h>
namespace zeus
{
using std::ratio;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::system_clock;
using filetime_duration = duration<int64_t, ratio<1, 10'000'000>>;
constexpr duration<int64_t>           kntToUnixEpoch {INT64_C(-11644473600)};
std::chrono::system_clock::time_point FiletimeToSystemTime(const FILETIME& time)
{
    const filetime_duration asDuration {static_cast<int64_t>((static_cast<uint64_t>(time.dwHighDateTime) << 32) | time.dwLowDateTime)};
    const auto              withUnixEpoch = asDuration + kntToUnixEpoch;
    return system_clock::time_point {duration_cast<system_clock::duration>(withUnixEpoch)};
}

std::chrono::system_clock::time_point FiletimeToSystemTime(const LARGE_INTEGER& time)
{
    FILETIME filetime;
    filetime.dwLowDateTime  = time.LowPart;
    filetime.dwHighDateTime = time.HighPart;
    return FiletimeToSystemTime(filetime);
}

}

#endif