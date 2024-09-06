#ifdef _WIN32
#include "zeus/foundation/system/win/file_digital_sign.h"
#include <algorithm>
#include <cassert>
#include <Windows.h>
#include <wincrypt.h>
#include "zeus/foundation/core/system_error.h"
#include "zeus/foundation/resource/auto_release.h"
#include "zeus/foundation/string/string_utils.h"
#include "zeus/foundation/string/charset_utils.h"

#pragma comment(lib, "crypt32.lib")

namespace zeus
{

namespace
{
#define ENCODING (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)

const char *OID_NESTED_SIGNATURE = "1.3.6.1.4.1.311.2.4.1";

std::optional<std::string> GetCertNameString(PCCERT_CONTEXT certContext, DWORD type, DWORD flag)
{
    assert(certContext);

    auto length = ::CertGetNameStringW(certContext, type, flag, nullptr, nullptr, 0); //get data size
    if (!length)
    {
        return std::nullopt;
    }
    std::unique_ptr<wchar_t[]> buffer = std::make_unique<wchar_t[]>(length * sizeof(wchar_t));
    std::memset(buffer.get(), 0, length * sizeof(wchar_t));
    if (::CertGetNameStringW(certContext, type, flag, nullptr, buffer.get(), length))
    {
        return zeus::CharsetUtils::UnicodeToUTF8(buffer.get());
    }
    return std::nullopt;
}
} // namespace

struct WinDigtalSignImpl
{
    std::shared_ptr<const CERT_CONTEXT> certContext;
};

struct WinFileDigtalSignImpl
{
    std::vector<WinDigtalSign> datas;
};

WinDigtalSign::WinDigtalSign() : _impl(std::make_unique<WinDigtalSignImpl>())
{
}
WinDigtalSign::WinDigtalSign(WinDigtalSign &&other) noexcept : _impl(std::move(other._impl))
{
}
WinDigtalSign &WinDigtalSign::operator=(WinDigtalSign &&other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}
WinDigtalSign::~WinDigtalSign()
{
}

std::optional<std::string> WinDigtalSign::GetSubjectName() const
{
    return GetCertNameString(_impl->certContext.get(), CERT_NAME_SIMPLE_DISPLAY_TYPE, 0);
}
std::optional<std::string> WinDigtalSign::GetSerialNumber() const
{
    if (!_impl->certContext->pCertInfo)
    {
        return std::nullopt;
    }

    const DWORD size = _impl->certContext->pCertInfo->SerialNumber.cbData;
    if (!size)
    {
        return std::nullopt;
    }
    std::string serialNumber;
    serialNumber.reserve(size * 2);
    for (DWORD index = 0; index < size; ++index)
    {
        std::string item = BytesToHexString(_impl->certContext->pCertInfo->SerialNumber.pbData + index, 1, false);
        std::reverse(item.begin(), item.end());
        serialNumber.append(item);
    }
    std::reverse(serialNumber.begin(), serialNumber.end());
    return serialNumber;
}
std::optional<std::string> WinDigtalSign::GetIssuerName() const
{
    return GetCertNameString(_impl->certContext.get(), CERT_NAME_SIMPLE_DISPLAY_TYPE, CERT_NAME_ISSUER_FLAG);
}

zeus::WinFileDigtalSign::WinFileDigtalSign() : _impl(std::make_unique<WinFileDigtalSignImpl>())
{
}

zeus::WinFileDigtalSign::~WinFileDigtalSign()
{
}

WinFileDigtalSign::WinFileDigtalSign(WinFileDigtalSign &&other) noexcept : _impl(std::move(other._impl))
{
}

WinFileDigtalSign &WinFileDigtalSign::operator=(WinFileDigtalSign &&other) noexcept
{
    if (this != &other)
    {
        _impl = std::move(other._impl);
    }
    return *this;
}

const std::vector<WinDigtalSign> &WinFileDigtalSign::GetDigtalSigns() const
{
    return _impl->datas;
}

zeus::expected<WinFileDigtalSign, std::error_code> WinFileDigtalSign::Load(const std::filesystem::path &path, bool nested)
{
    DWORD      encoding    = 0;
    DWORD      contentType = 0;
    DWORD      formatType  = 0;
    HCERTSTORE store       = nullptr;
    HCRYPTMSG  msg         = nullptr;

    zeus::AutoRelease release(
        [&store, &msg]()
        {
            if (store)
            {
                ::CertCloseStore(store, 0);
            }
            if (msg)
            {
                ::CryptMsgClose(msg);
            }
        }
    );
    if (!::CryptQueryObject(
            CERT_QUERY_OBJECT_FILE, path.c_str(), CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED, CERT_QUERY_FORMAT_FLAG_BINARY, 0, &encoding,
            &contentType, &formatType, &store, &msg, nullptr
        ))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    DWORD signerSize = 0;
    if (!::CryptMsgGetParam(msg, CMSG_SIGNER_INFO_PARAM, 0, nullptr, &signerSize))
    {
        return zeus::unexpected(GetLastSystemError());
    }
    auto signerInfo =
        std::unique_ptr<CMSG_SIGNER_INFO, decltype(&LocalFree)>(reinterpret_cast<PCMSG_SIGNER_INFO>(LocalAlloc(LPTR, signerSize)), &LocalFree);
    if (!::CryptMsgGetParam(msg, CMSG_SIGNER_INFO_PARAM, 0, signerInfo.get(), &signerSize) && signerInfo)
    {
        return zeus::unexpected(GetLastSystemError());
    }
    WinFileDigtalSign fileDigtalSign;

    CERT_INFO certInfo    = {};
    certInfo.Issuer       = signerInfo->Issuer;
    certInfo.SerialNumber = signerInfo->SerialNumber;
    const auto *context   = ::CertFindCertificateInStore(store, ENCODING, 0, CERT_FIND_SUBJECT_CERT, &certInfo, NULL);
    if (context)
    {
        WinDigtalSign sign;
        sign._impl->certContext = std::shared_ptr<const CERT_CONTEXT>(context, CertFreeCertificateContext);
        fileDigtalSign._impl->datas.emplace_back(std::move(sign));
    }

    if (nested)
    {
        //查找是否有嵌套的证书，exe允许多个签名，因为只找到两签名的测试文件，所以2个以上的签名不确定是否可以此代码是否支持
        for (DWORD i = 0U; i < signerInfo->UnauthAttrs.cAttr; ++i)
        {
            if (0 != std::strcmp(signerInfo->UnauthAttrs.rgAttr[i].pszObjId, OID_NESTED_SIGNATURE))
            {
                continue;
            }

            HCRYPTMSG         nestedMsg = CryptMsgOpenToDecode(ENCODING, 0, 0, 0, nullptr, 0);
            zeus::AutoRelease releaseMsg(
                [&nestedMsg]()
                {
                    if (nestedMsg)
                    {
                        ::CryptMsgClose(nestedMsg);
                    }
                }
            );
            if (!nestedMsg)
            {
                continue;
            }

            if (!CryptMsgUpdate(
                    nestedMsg, signerInfo->UnauthAttrs.rgAttr[i].rgValue->pbData, signerInfo->UnauthAttrs.rgAttr[i].rgValue->cbData, TRUE
                ))
            {
                continue;
            }

            if (!CryptMsgGetParam(nestedMsg, CMSG_SIGNER_INFO_PARAM, 0, nullptr, &signerSize))
            {
                continue;
            }
            auto nestedSignerInfo = std::unique_ptr<CMSG_SIGNER_INFO, decltype(&LocalFree)>(
                reinterpret_cast<PCMSG_SIGNER_INFO>(LocalAlloc(LPTR, signerSize)), &LocalFree
            );
            if (!::CryptMsgGetParam(nestedMsg, CMSG_SIGNER_INFO_PARAM, 0, nestedSignerInfo.get(), &signerSize) && nestedSignerInfo)
            {
                continue;
            }
            HCERTSTORE        nestedStore = CertOpenStore(CERT_STORE_PROV_MSG, ENCODING, 0, 0, nestedMsg);
            zeus::AutoRelease nestedStoreMsg(
                [&nestedStore]()
                {
                    if (nestedStore)
                    {
                        ::CertCloseStore(nestedStore, 0);
                    }
                }
            );
            if (!nestedStore)
            {
                continue;
            }
            CERT_INFO nestedCertInfo    = {};
            nestedCertInfo.Issuer       = nestedSignerInfo->Issuer;
            nestedCertInfo.SerialNumber = nestedSignerInfo->SerialNumber;
            const auto *nestedContext   = ::CertFindCertificateInStore(nestedStore, ENCODING, 0, CERT_FIND_SUBJECT_CERT, &nestedCertInfo, nullptr);
            if (nestedContext)
            {
                WinDigtalSign sign;
                sign._impl->certContext = std::shared_ptr<const CERT_CONTEXT>(nestedContext, CertFreeCertificateContext);
                fileDigtalSign._impl->datas.emplace_back(std::move(sign));
            }
        }
    }
    return fileDigtalSign;
}

} // namespace zeus
#endif //_WIN32
