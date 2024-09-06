#ifdef _WIN32
#include "zeus/foundation/resource/win/com_init.h"
#include <cassert>
#include <ObjBase.h>
namespace zeus
{
struct ComInitImpl
{
    ComInit::ComType type;
    std::thread::id  threadId;
};

ComInit::ComInit(ComType type) : _impl(std::make_unique<ComInitImpl>())
{
    if (ComType::NEUTRAL == type || ComType::NONE_INIT == type)
    {
        type = ComType::MULTITHREADED;
    }
    _impl->threadId = std::this_thread::get_id();
    if (type == ComType::APARTMENTTHREADED)
    {
        HRESULT hres = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        _impl->type  = ComType::APARTMENTTHREADED;
        if (hres == RPC_E_CHANGED_MODE)
        {
            hres        = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
            _impl->type = ComType::MULTITHREADED;
        }
    }
    else
    {
        HRESULT hres = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
        _impl->type  = ComType::MULTITHREADED;
        if (hres == RPC_E_CHANGED_MODE)
        {
            hres        = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
            _impl->type = ComType::APARTMENTTHREADED;
        }
    }
}
ComInit::~ComInit()
{
    assert(_impl->threadId == std::this_thread::get_id());

    CoUninitialize();
}
ComInit::ComInit(ComInit&& other) noexcept : _impl(std::move(other._impl))
{
}

ComInit& ComInit::operator=(ComInit&& other) noexcept
{
    if (this != &other)
    {
        _impl.swap(other._impl);
    }
    return *this;
}
ComInit::ComType ComInit::CurrentType()
{
    return _impl->type;
}
std::thread::id ComInit::CurrentThreadId()
{
    return _impl->threadId;
}
ComInit::ComType ComInit::GetCurrentComType()
{
    APTTYPE          type = APTTYPE::APTTYPE_CURRENT;
    APTTYPEQUALIFIER temp;
    CoGetApartmentType(&type, &temp);
    switch (type)
    {
    case APTTYPE::APTTYPE_CURRENT:
        return ComInit::ComType::NONE_INIT;
    case APTTYPE::APTTYPE_MAINSTA:
    case APTTYPE::APTTYPE_STA:
        return ComInit::ComType::APARTMENTTHREADED;
    case APTTYPE::APTTYPE_MTA:
        return ComInit::ComType::MULTITHREADED;
    default:
        return ComInit::ComType::NEUTRAL;
    }
}
}; // namespace zeus
#endif
