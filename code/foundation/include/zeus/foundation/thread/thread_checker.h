#pragma once
#include <memory>

namespace zeus
{
struct ThreadCheckerImpl;
class ThreadChecker
{
public:
    ThreadChecker(bool attach = true);
    ~ThreadChecker();
    ThreadChecker(const ThreadChecker&)            = delete;
    ThreadChecker(ThreadChecker&&)                 = delete;
    ThreadChecker& operator=(const ThreadChecker&) = delete;
    bool           IsCurrent() const;
    void           Detach();
private:
    std::unique_ptr<ThreadCheckerImpl> _impl;
};
}
#include "zeus/foundation/core/zeus_compatible.h"
