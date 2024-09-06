#include "zeus/foundation/dep_inject/injected_object.h"
namespace zeus
{
struct InjectedObjectImpl
{
    std::list<std::function<bool(ObjectConstainer& container)>> injectTaskList;
    std::list<std::function<void()>>                            uninjectTaskList;
};
InjectedObject::InjectedObject() : _impl(std::make_unique<InjectedObjectImpl>())
{
}
InjectedObject::~InjectedObject()
{
}
bool InjectedObject::Inject(ObjectConstainer& container)
{
    bool result = true;
    for (auto iter = _impl->injectTaskList.begin(); iter != _impl->injectTaskList.end();)
    {
        if ((*iter)(container))
        {
            iter = _impl->injectTaskList.erase(iter);
        }
        else
        {
            result &= false;
            ++iter;
        }
    }
    return result;
}

void InjectedObject::Uninject()
{
    for (auto task : _impl->uninjectTaskList)
    {
        task();
    }
    _impl->uninjectTaskList.clear();
}

void InjectedObject::AddInjectTask(std::function<bool(ObjectConstainer& container)>&& task)
{
    _impl->injectTaskList.emplace_back(std::move(task));
}
void InjectedObject::AddUninjectTask(std::function<void()>&& task)
{
    _impl->uninjectTaskList.emplace_back(std::move(task));
}
} // namespace zeus