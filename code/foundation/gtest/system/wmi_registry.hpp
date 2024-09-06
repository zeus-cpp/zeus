#pragma once
#include <zeus/foundation/resource/win/com_init.h>
#include <Wbemidl.h>
#include <atlbase.h>

#pragma comment(lib, "wbemuuid.lib")

namespace Ares
{
class WinRegistry
{
#define CHECK_RES     \
    if (res)          \
    {                 \
        return false; \
    }
#define ARES_RELEASE                    \
    if (pInParamsDefinition)            \
    {                                   \
        pInParamsDefinition->Release(); \
    }                                   \
    if (pInParams)                      \
    {                                   \
        pInParams->Release();           \
    }                                   \
    if (pOutParams)                     \
    {                                   \
        pOutParams->Release();          \
    }

public:
    WinRegistry(HKEY rootKey, const std::wstring &subKey, int osBit = 32) : _rootKey(rootKey), _subKey(subKey), _osBit(osBit) {}
    ~WinRegistry()
    {
        if (_pClass)
        {
            _pClass->Release();
        };
        if (_wbemC)
        {
            _wbemC->Release();
        };
        if (_wbemS)
        {
            _wbemS->Release();
        };
        if (_wbemL)
        {
            _wbemL->Release();
        };
    }

    bool Init()
    {
        VARIANT vArchitecture;
        vArchitecture.vt      = VT_I4;
        vArchitecture.uintVal = _osBit;
        HRESULT res;
        res = CoCreateInstance(CLSID_WbemContext, NULL, CLSCTX_INPROC_SERVER, IID_IWbemContext, reinterpret_cast<LPVOID *>(&_wbemC));
        CHECK_RES;
        res = _wbemC->SetValue(BSTR(L"__ProviderArchitecture"), 0, &vArchitecture);
        CHECK_RES;

        res = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<LPVOID *>(&_wbemL));
        CHECK_RES;
        res = _wbemL->ConnectServer(CComBSTR(zeus::CharsetUtils::UTF8ToUnicode("ROOT\\CIMV2").c_str()), NULL, NULL, NULL, 0, NULL, NULL, &_wbemS);
        CHECK_RES;
        res = CoSetProxyBlanket(
            _wbemS, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE
        );
        CHECK_RES;

        res = _wbemS->GetObjectW(const_cast<BSTR>(L"StdRegProv"), 0, NULL, &_pClass, NULL);
        CHECK_RES;

        return true;
    }

    void SetString(const std::wstring &key, const std::wstring &value)
    {
        std::wstring method = L"SetStringValue";

        HRESULT           res;
        IWbemClassObject *pInParamsDefinition = nullptr;
        res                                   = _pClass->GetMethod(method.c_str(), 0, &pInParamsDefinition, NULL);
        if (FAILED(res))
        {
            return;
        }

        IWbemClassObject *pInParams  = nullptr;
        IWbemClassObject *pOutParams = nullptr;
        res                          = pInParamsDefinition->SpawnInstance(0, &pInParams);
        if (FAILED(res))
        {
            return;
        }

        VARIANT vKey;

        vKey.vt      = VT_I4;
        vKey.uintVal = (uint32_t) _rootKey;
        pInParams->Put(L"hDefKey", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(_subKey.c_str());
        pInParams->Put(L"sSubKeyName", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(key.c_str());
        pInParams->Put(L"sValueName", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(value.c_str());
        pInParams->Put(L"sValue", 0, &vKey, 0);

        res = _wbemS->ExecMethod(const_cast<BSTR>(L"StdRegProv"), BSTR(method.c_str()), 0, _wbemC, pInParams, &pOutParams, NULL);
        if (FAILED(res))
        {
            return;
        }

        ARES_RELEASE;
    }
    std::wstring GetString(const std::wstring &key)
    {
        std::wstring method = L"GetStringValue";
        std::wstring ret    = L"";

        HRESULT           res;
        IWbemClassObject *pInParamsDefinition = nullptr;
        res                                   = _pClass->GetMethod(method.c_str(), 0, &pInParamsDefinition, NULL);
        if (FAILED(res))
        {
            return ret;
        }

        IWbemClassObject *pInParams  = nullptr;
        IWbemClassObject *pOutParams = nullptr;
        res                          = pInParamsDefinition->SpawnInstance(0, &pInParams);
        if (FAILED(res))
        {
            return ret;
        }

        VARIANT vKey;

        vKey.vt      = VT_I4;
        vKey.uintVal = (uint32_t) _rootKey;
        pInParams->Put(L"hDefKey", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(_subKey.c_str());
        pInParams->Put(L"sSubKeyName", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(key.c_str());
        pInParams->Put(L"sValueName", 0, &vKey, 0);

        res = _wbemS->ExecMethod(const_cast<BSTR>(L"StdRegProv"), BSTR(method.c_str()), 0, _wbemC, pInParams, &pOutParams, NULL);
        if (FAILED(res))
        {
            return ret;
        }

        vKey.vt = VT_LPWSTR;
        pOutParams->Get(BSTR(L"sValue"), 0, &vKey, NULL, NULL);
        ret = (wchar_t *) vKey.pbstrVal;

        ARES_RELEASE;

        return ret;
    }

    void SetStringExpand(const std::wstring &key, const std::wstring &value)
    {
        std::wstring method = L"SetExpandedStringValue";

        HRESULT           res;
        IWbemClassObject *pInParamsDefinition = nullptr;
        res                                   = _pClass->GetMethod(method.c_str(), 0, &pInParamsDefinition, NULL);
        if (FAILED(res))
        {
            return;
        }

        IWbemClassObject *pInParams  = nullptr;
        IWbemClassObject *pOutParams = nullptr;
        res                          = pInParamsDefinition->SpawnInstance(0, &pInParams);
        if (FAILED(res))
        {
            return;
        }

        VARIANT vKey;

        vKey.vt      = VT_I4;
        vKey.uintVal = (uint32_t) _rootKey;
        pInParams->Put(L"hDefKey", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(_subKey.c_str());
        pInParams->Put(L"sSubKeyName", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(key.c_str());
        pInParams->Put(L"sValueName", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(value.c_str());
        pInParams->Put(L"sValue", 0, &vKey, 0);

        res = _wbemS->ExecMethod(const_cast<BSTR>(L"StdRegProv"), BSTR(method.c_str()), 0, _wbemC, pInParams, &pOutParams, NULL);
        if (FAILED(res))
        {
            return;
        }

        ARES_RELEASE;
    }
    std::wstring GetStringExpand(const std::wstring &key)
    {
        std::wstring method = L"GetExpandedStringValue";
        std::wstring ret    = L"";

        HRESULT           res;
        IWbemClassObject *pInParamsDefinition = nullptr;
        res                                   = _pClass->GetMethod(method.c_str(), 0, &pInParamsDefinition, NULL);
        if (FAILED(res))
        {
            return ret;
        }

        IWbemClassObject *pInParams  = nullptr;
        IWbemClassObject *pOutParams = nullptr;
        res                          = pInParamsDefinition->SpawnInstance(0, &pInParams);
        if (FAILED(res))
        {
            return ret;
        }

        VARIANT vKey;

        vKey.vt      = VT_I4;
        vKey.uintVal = (uint32_t) _rootKey;
        pInParams->Put(L"hDefKey", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(_subKey.c_str());
        pInParams->Put(L"sSubKeyName", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(key.c_str());
        pInParams->Put(L"sValueName", 0, &vKey, 0);

        res = _wbemS->ExecMethod(const_cast<BSTR>(L"StdRegProv"), BSTR(method.c_str()), 0, _wbemC, pInParams, &pOutParams, NULL);
        if (FAILED(res))
        {
            return ret;
        }

        vKey.vt = VT_LPWSTR;
        pOutParams->Get(BSTR(L"sValue"), 0, &vKey, NULL, NULL);
        ret = (wchar_t *) vKey.pbstrVal;

        ARES_RELEASE;

        return ret;
    }

    void SetInt(const std::wstring &key, int value)
    {
        std::wstring method = L"SetDWORDValue";

        HRESULT           res;
        IWbemClassObject *pInParamsDefinition = nullptr;
        res                                   = _pClass->GetMethod(method.c_str(), 0, &pInParamsDefinition, NULL);
        if (FAILED(res))
        {
            return;
        }

        IWbemClassObject *pInParams  = nullptr;
        IWbemClassObject *pOutParams = nullptr;
        res                          = pInParamsDefinition->SpawnInstance(0, &pInParams);
        if (FAILED(res))
        {
            return;
        }

        VARIANT vKey;

        vKey.vt      = VT_I4;
        vKey.uintVal = (uint32_t) _rootKey;
        pInParams->Put(L"hDefKey", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(_subKey.c_str());
        pInParams->Put(L"sSubKeyName", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(key.c_str());
        pInParams->Put(L"sValueName", 0, &vKey, 0);

        vKey.vt      = VT_I4;
        vKey.uintVal = value;
        pInParams->Put(L"uValue", 0, &vKey, 0);

        res = _wbemS->ExecMethod(const_cast<BSTR>(L"StdRegProv"), BSTR(method.c_str()), 0, _wbemC, pInParams, &pOutParams, NULL);
        if (FAILED(res))
        {
            return;
        }

        ARES_RELEASE;
    }
    int GetInt(const std::wstring &key)
    {
        std::wstring method = L"GetDWORDValue";
        int          ret    = 0;

        HRESULT           res;
        IWbemClassObject *pInParamsDefinition = nullptr;
        res                                   = _pClass->GetMethod(method.c_str(), 0, &pInParamsDefinition, NULL);
        if (FAILED(res))
        {
            return ret;
        }

        IWbemClassObject *pInParams  = nullptr;
        IWbemClassObject *pOutParams = nullptr;
        res                          = pInParamsDefinition->SpawnInstance(0, &pInParams);
        if (FAILED(res))
        {
            return ret;
        }

        VARIANT vKey;

        vKey.vt      = VT_I4;
        vKey.uintVal = (uint32_t) _rootKey;
        pInParams->Put(L"hDefKey", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(_subKey.c_str());
        pInParams->Put(L"sSubKeyName", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(key.c_str());
        pInParams->Put(L"sValueName", 0, &vKey, 0);

        res = _wbemS->ExecMethod(const_cast<BSTR>(L"StdRegProv"), BSTR(method.c_str()), 0, _wbemC, pInParams, &pOutParams, NULL);
        if (FAILED(res))
        {
            return ret;
        }

        vKey.vt = VT_I4;
        pOutParams->Get(BSTR(L"uValue"), 0, &vKey, NULL, NULL);
        ret = vKey.ulVal;

        ARES_RELEASE;

        return ret;
    }

    void SetInt64(const std::wstring &key, long long value)
    {
        std::wstring method = L"SetQWORDValue";

        HRESULT           res;
        IWbemClassObject *pInParamsDefinition = nullptr;
        res                                   = _pClass->GetMethod(method.c_str(), 0, &pInParamsDefinition, NULL);
        if (FAILED(res))
        {
            return;
        }

        IWbemClassObject *pInParams  = nullptr;
        IWbemClassObject *pOutParams = nullptr;
        res                          = pInParamsDefinition->SpawnInstance(0, &pInParams);
        if (FAILED(res))
        {
            return;
        }

        VARIANT vKey;

        vKey.vt      = VT_I4;
        vKey.uintVal = (uint32_t) _rootKey;
        pInParams->Put(L"hDefKey", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(_subKey.c_str());
        pInParams->Put(L"sSubKeyName", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(key.c_str());
        pInParams->Put(L"sValueName", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(std::to_string(value).c_str());
        pInParams->Put(L"uValue", 0, &vKey, 0);

        res = _wbemS->ExecMethod(const_cast<BSTR>(L"StdRegProv"), BSTR(method.c_str()), 0, _wbemC, pInParams, &pOutParams, NULL);
        if (FAILED(res))
        {
            return;
        }

        ARES_RELEASE;
    }
    long long GetInt64(const std::wstring &key)
    {
        std::wstring method = L"GetQWORDValue";
        long long    ret    = 0;

        HRESULT           res;
        IWbemClassObject *pInParamsDefinition = nullptr;
        res                                   = _pClass->GetMethod(method.c_str(), 0, &pInParamsDefinition, NULL);
        if (FAILED(res))
        {
            return ret;
        }

        IWbemClassObject *pInParams  = nullptr;
        IWbemClassObject *pOutParams = nullptr;
        res                          = pInParamsDefinition->SpawnInstance(0, &pInParams);
        if (FAILED(res))
        {
            return ret;
        }

        VARIANT vKey;

        vKey.vt      = VT_I4;
        vKey.uintVal = (uint32_t) _rootKey;
        pInParams->Put(L"hDefKey", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(_subKey.c_str());
        pInParams->Put(L"sSubKeyName", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(key.c_str());
        pInParams->Put(L"sValueName", 0, &vKey, 0);

        res = _wbemS->ExecMethod(const_cast<BSTR>(L"StdRegProv"), BSTR(method.c_str()), 0, _wbemC, pInParams, &pOutParams, NULL);
        if (FAILED(res))
        {
            return ret;
        }

        pOutParams->Get(BSTR(L"uValue"), 0, &vKey, NULL, NULL);
        ret = std::stoll(vKey.bstrVal);

        ARES_RELEASE;

        return ret;
    }

    void SetBinary(const std::wstring &key, const std::vector<char> &value)
    {
        std::wstring method = L"SetBinaryValue";

        HRESULT           res;
        IWbemClassObject *pInParamsDefinition = nullptr;
        res                                   = _pClass->GetMethod(method.c_str(), 0, &pInParamsDefinition, NULL);
        if (FAILED(res))
        {
            return;
        }

        IWbemClassObject *pInParams  = nullptr;
        IWbemClassObject *pOutParams = nullptr;
        res                          = pInParamsDefinition->SpawnInstance(0, &pInParams);
        if (FAILED(res))
        {
            return;
        }

        VARIANT vKey;

        vKey.vt      = VT_I4;
        vKey.uintVal = (uint32_t) _rootKey;
        pInParams->Put(L"hDefKey", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(_subKey.c_str());
        pInParams->Put(L"sSubKeyName", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(key.c_str());
        pInParams->Put(L"sValueName", 0, &vKey, 0);

        SAFEARRAY     *psa;
        SAFEARRAYBOUND rgsabound[1] = {value.size(), 0};
        psa                         = SafeArrayCreate(VT_UI1, 1, rgsabound);
        for (LONG i = 0; i < static_cast<LONG>(value.size()); ++i)
        {
            SafeArrayPutElement(psa, &i, (void *) &value[i]);
        }

        //vKey.vt = VT_SAFEARRAY;
        vKey.vt     = VT_ARRAY | VT_UI1;
        vKey.parray = psa;
        pInParams->Put(L"uValue", 0, &vKey, 0);

        res = _wbemS->ExecMethod(const_cast<BSTR>(L"StdRegProv"), BSTR(method.c_str()), 0, _wbemC, pInParams, &pOutParams, NULL);
        if (FAILED(res))
        {
            return;
        }

        ARES_RELEASE;
    }
    std::vector<char> GetBinary(const std::wstring &key)
    {
        std::wstring      method = L"GetBinaryValue";
        std::vector<char> ret;

        HRESULT           res;
        IWbemClassObject *pInParamsDefinition = nullptr;
        res                                   = _pClass->GetMethod(method.c_str(), 0, &pInParamsDefinition, NULL);
        if (FAILED(res))
        {
            return ret;
        }

        IWbemClassObject *pInParams  = nullptr;
        IWbemClassObject *pOutParams = nullptr;
        res                          = pInParamsDefinition->SpawnInstance(0, &pInParams);
        if (FAILED(res))
        {
            return ret;
        }

        VARIANT vKey;

        vKey.vt      = VT_I4;
        vKey.uintVal = (uint32_t) _rootKey;
        pInParams->Put(L"hDefKey", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(_subKey.c_str());
        pInParams->Put(L"sSubKeyName", 0, &vKey, 0);

        vKey.vt      = VT_BSTR;
        vKey.bstrVal = BSTR(key.c_str());
        pInParams->Put(L"sValueName", 0, &vKey, 0);

        res = _wbemS->ExecMethod(const_cast<BSTR>(L"StdRegProv"), BSTR(method.c_str()), 0, _wbemC, pInParams, &pOutParams, NULL);
        if (FAILED(res))
        {
            return ret;
        }

        pOutParams->Get(BSTR(L"uValue"), 0, &vKey, NULL, NULL);
        void *data = nullptr;
        SafeArrayAccessData(vKey.parray, &data);
        for (auto i = 0u; i < vKey.parray->rgsabound->cElements; ++i)
        {
            ret.push_back(reinterpret_cast<char *>(data)[i]);
        }
        SafeArrayUnaccessData(vKey.parray);

        ARES_RELEASE;

        return ret;
    }

private:
    IWbemServices    *_wbemS  = nullptr;
    IWbemLocator     *_wbemL  = nullptr;
    IWbemContext     *_wbemC  = nullptr;
    IWbemClassObject *_pClass = nullptr;

    HKEY         _rootKey;
    std::wstring _subKey;
    int          _osBit;

    zeus::ComInit _com;
};
}; // namespace Ares
