#ifdef _WIN32
#include "zeus/foundation/system/win/wmi.h"
#include <mutex>
#include <cassert>
#include <Objbase.h>
#include <atlbase.h>
#include <Wbemidl.h>
#include "zeus/foundation/core/win/com_error.h"
#include "zeus/foundation/resource/win/com_init.h"
#include "zeus/foundation/string/charset_utils.h"
#include "zeus/foundation/thread/thread_checker.h"
#pragma comment(lib, "wbemuuid.lib")

using namespace nlohmann;

namespace zeus
{

namespace
{
void ParseObject(CComPtr<IWbemClassObject> pclsObj, json &obj);
void ParseknowTypeItem(const CComBSTR &bstrName, const CComVariant &value, json &object);
void ParseObject(CComPtr<IWbemClassObject> pclsObj, json &obj)
{
    pclsObj->BeginEnumeration(WBEM_FLAG_LOCAL_ONLY);
    do
    {
        CComBSTR    bstrName;
        CComVariant value;
        CIMTYPE     type    = 0;
        LONG        lFlavor = 0;
        if (WBEM_S_NO_ERROR != pclsObj->Next(0, &bstrName, &value, &type, &lFlavor))
        {
            break;
        }
        if (value.vt != VT_UNKNOWN)
        {
            ParseknowTypeItem(bstrName, value, obj);
        }
    }
    while (true);
    pclsObj->EndEnumeration();
}

void ParseknowTypeItem(const CComBSTR &bstrName, const CComVariant &value, json &object)
{
    std::string name = zeus::CharsetUtils::UnicodeToUTF8(std::wstring(bstrName.m_str));
    if (value.vt & VT_ARRAY)
    {
        VARTYPE type = VT_EMPTY;
        if (S_OK == SafeArrayGetVartype(value.parray, &type))
        {
            const size_t count = value.parray->rgsabound->cElements;
            void HUGEP  *data;
            if (S_OK == SafeArrayAccessData(value.parray, &data))
            {
                for (size_t i = 0; i < count; i++)
                {
                    switch (type)
                    {
                    case VT_BSTR:
                        object[name].emplace_back(zeus::CharsetUtils::UnicodeToUTF8(std::wstring(static_cast<BSTR *>(data)[i])));
                        break;
                    case VT_I1:
                        object[name].emplace_back(static_cast<CHAR *>(data)[i]);
                        break;
                    case VT_I2:
                        object[name].emplace_back(static_cast<SHORT *>(data)[i]);
                        break;
                    case VT_I4:
                        object[name].emplace_back(static_cast<int32_t>(static_cast<LONG *>(data)[i]));
                        break;
                    case VT_I8:
                        object[name].emplace_back(static_cast<LONGLONG *>(data)[i]);
                        break;
                    case VT_INT:
                        object[name].emplace_back(static_cast<INT *>(data)[i]);
                        break;
                    case VT_UI1:
                        object[name].emplace_back(static_cast<BYTE *>(data)[i]);
                        break;
                    case VT_UI2:
                        object[name].emplace_back(static_cast<USHORT *>(data)[i]);
                        break;
                    case VT_UI4:
                        object[name].emplace_back(static_cast<size_t>(static_cast<ULONG *>(data)[i]));
                        break;
                    case VT_UI8:
                        object[name].emplace_back(static_cast<ULONGLONG *>(data)[i]);
                        break;
                    case VT_UINT:
                        object[name].emplace_back(static_cast<UINT *>(data)[i]);
                        break;
                    case VT_BOOL:
                        object[name].emplace_back(static_cast<VARIANT_BOOL *>(data)[i]);
                        break;
                    case VT_R4:
                        object[name].emplace_back(static_cast<FLOAT *>(data)[i]);
                        break;
                    case VT_R8:
                        object[name].emplace_back(static_cast<DOUBLE *>(data)[i]);
                        break;
                    default:
                        break;
                    }
                }
                SafeArrayUnaccessData(value.parray);
            }
        }
    }
    else
    {
        switch (value.vt)
        {
        case VT_BSTR:
            object[name] = zeus::CharsetUtils::UnicodeToUTF8(std::wstring(value.bstrVal));
            break;
        case VT_I1:
            object[name] = static_cast<int>(value.cVal);
            break;
        case VT_I2:
            object[name] = static_cast<int>(value.iVal);
            break;
        case VT_I4:
            object[name] = static_cast<int>(value.lVal);
            break;
        case VT_I8:
            object[name] = static_cast<int64_t>(value.llVal);
            break;
        case VT_INT:
            object[name] = static_cast<int>(value.intVal);
            break;
        case VT_UI1:
            object[name] = static_cast<unsigned int>(value.bVal);
            break;
        case VT_UI2:
            object[name] = static_cast<unsigned int>(value.uiVal);
            break;
        case VT_UI4:
            object[name] = static_cast<unsigned int>(value.ulVal);
            break;
        case VT_UI8:
            object[name] = static_cast<uint64_t>(value.ullVal);
            break;
        case VT_UINT:
            object[name] = static_cast<unsigned int>(value.uintVal);
            break;
        case VT_BOOL:
            object[name] = static_cast<bool>(value.boolVal);
            break;
        case VT_R4:
            object[name] = static_cast<bool>(value.fltVal);
            break;
        case VT_R8:
            object[name] = static_cast<bool>(value.dblVal);
            break;
        default:
            return;
        }
    }
}
} // namespace

struct WinWMIImpl
{
    std::unique_ptr<ComInit> comInit;
    IStream                 *stream = nullptr;
    ThreadChecker            checker;
    std::mutex               mutex;
};

WinWMI::WinWMI() : _impl(std::make_unique<WinWMIImpl>())
{
}
WinWMI::WinWMI(WinWMI &&other) noexcept : _impl(std::move(other._impl))
{
}
WinWMI &WinWMI::operator=(WinWMI &&other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
WinWMI::~WinWMI()
{
    if (_impl)
    {
        CoReleaseMarshalData(_impl->stream);
        _impl->stream->Release();
        _impl->stream = nullptr;
    }
}

zeus::expected<nlohmann::json, std::error_code> WinWMI::Query(const std::string &wql)
{
    json result;

    HRESULT       hr = S_OK;
    LARGE_INTEGER move;
    LISet32(move, 0);
    ULARGE_INTEGER           newPosition;
    std::unique_ptr<ComInit> init;
    CComPtr<IWbemServices>   svc = nullptr;
    {
        //只对IStream的加锁
        std::unique_lock<std::mutex> lock(_impl->mutex);
        //每次操作前把IStream位置还原
        hr = _impl->stream->Seek(move, STREAM_SEEK_SET, &newPosition);
        _impl->stream->AddRef();
        hr = CoGetInterfaceAndReleaseStream(_impl->stream, IID_IWbemServices, reinterpret_cast<LPVOID *>(&svc));
        if (CO_E_NOTINITIALIZED == hr)
        {
            init = std::make_unique<ComInit>(ComInit::ComType::APARTMENTTHREADED);
            //因为CoGetInterfaceAndReleaseStream内部哪怕失败了也会调用Release，所以每次调用前必须都先增加引用计数
            _impl->stream->AddRef();
            hr = CoGetInterfaceAndReleaseStream(_impl->stream, IID_IWbemServices, reinterpret_cast<LPVOID *>(&svc));
        }
        if (!SUCCEEDED(hr))
        {
            return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
        }
    }
    CComPtr<IEnumWbemClassObject> pEnumerator;
    //只有当某些线程模式组合时需要设置，但是会导致逻辑复杂，每次都设置并不影响
    CoSetProxyBlanket(svc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
    hr = svc->ExecQuery(CComBSTR("WQL"), CComBSTR(wql.c_str()), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator);
    if (SUCCEEDED(hr))
    {
        ULONG ret = 0;

        while (pEnumerator)
        {
            CComPtr<IWbemClassObject> pclsObj;
            hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &ret);
            if (ret && SUCCEEDED(hr))
            {
                json data;
                ParseObject(pclsObj, data);
                result.emplace_back(std::move(data));
            }
            else
            {
                break;
            }
        }
        return result;
    }
    else
    {
        return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
    }
}
zeus::expected<WinWMI, std::error_code> WinWMI::Create(const std::string &nameSpace)
{
    HRESULT hr = S_OK;
    do
    {
        auto                   comInit = std::make_unique<ComInit>(ComInit::ComType::MULTITHREADED);
        //后面全部通过IStream操作，这里的对象初始化完就可以释放了
        CComPtr<IWbemLocator>  loc;
        CComPtr<IWbemServices> svc;
        hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<LPVOID *>(&loc));
        if (!SUCCEEDED(hr))
        {
            break;
        }
        hr = loc->ConnectServer(CComBSTR(zeus::CharsetUtils::UTF8ToUnicode(nameSpace).c_str()), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &svc);
        if (!SUCCEEDED(hr))
        {
            break;
        }
        ULONG size = 0;
        hr         = CoGetMarshalSizeMax(&size, IID_IWbemServices, svc, MSHCTX_INPROC, nullptr, MSHLFLAGS_TABLESTRONG);
        if (!SUCCEEDED(hr))
        {
            break;
        }
        HGLOBAL  global = GlobalAlloc(GMEM_MOVEABLE, size);
        IStream *stream = nullptr;
        hr              = CreateStreamOnHGlobal(global, TRUE, &stream);
        if (!SUCCEEDED(hr))
        {
            break;
        }
        hr = CoMarshalInterface(stream, IID_IWbemServices, svc, MSHCTX_INPROC, nullptr, MSHLFLAGS_TABLESTRONG);
        if (!SUCCEEDED(hr))
        {
            break;
        }
        WinWMI wmi;
        wmi._impl->comInit.swap(comInit);
        wmi._impl->stream = stream;
        return wmi;
    }
    while (false);
    return zeus::unexpected(std::error_code(hr, WinComCategory::ErrorCategory()));
}
} // namespace zeus
#endif
