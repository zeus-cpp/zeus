#include <gtest/gtest.h>
#include <zeus/foundation/core/random.h>
#include <zeus/foundation/crypt/uuid.h>
#include <zeus/foundation/crypt/md5_digest.h>
#include <zeus/foundation/crypt/raw_md5_digest.h>
#include <zeus/foundation/crypt/crc32_digest.h>
#include <zeus/foundation/crypt/sha1_digest.h>
#include <zeus/foundation/crypt/sha224_digest.h>
#include <zeus/foundation/crypt/sha384_digest.h>
#include <zeus/foundation/crypt/sha256_digest.h>
#include <zeus/foundation/crypt/sha512_digest.h>
#include <zeus/foundation/crypt/crc64_ecma182_digest.h>
#include <zeus/foundation/crypt/base64_encrypt.h>
#include <zeus/foundation/crypt/base64_decrypt.h>
#include <zeus/foundation/crypt/aes_encrypt.h>
#include <zeus/foundation/crypt/aes_decrypt.h>
#include <zeus/foundation/crypt/hmac_digest.h>
#include <zeus/foundation/string/string_utils.h>
#include <zeus/foundation/system/current_exe.h>
#include <zeus/foundation/file/file_wrapper.h>
#include "base64_longstring.h"

using namespace std;
using namespace zeus;
namespace fs = std::filesystem;

void TestDigest(BaseDigest &digest, const std::string &planText, const std::string &expectDigestText)
{
    auto dir  = zeus::CurrentExe::GetAppDir() / "crypt";
    auto path = dir / "digestTest";
    fs::remove_all(dir);
    fs::create_directories(dir);
    {
        digest.Update(planText);
        auto digestText = digest.ToString();
        EXPECT_EQ(expectDigestText, digestText);
        digest.Reset();
    }
    {
        digest.Update(planText.data(), planText.size());
        auto digestText = digest.ToString();
        EXPECT_EQ(expectDigestText, digestText);
        digest.Reset();
    }

    {
        auto file = FileWrapper::Create(path, FileWrapper::OpenMode::kReadWrite);
        file->Write(planText.data(), planText.size());
        file->Flush();
    }

    {
        digest.Update(path, true);
        auto digestText = digest.ToString();
        EXPECT_EQ(expectDigestText, digestText);
        digest.Reset();
    }

    {
        std::ifstream stream(path, std::ios::binary);
        digest.Update(stream);
        auto digestText = digest.ToString();
        EXPECT_EQ(expectDigestText, digestText);
        digest.Reset();
    }

    fs::remove(path);
}

#ifdef _WIN32
TEST(UUID, GUID)
{
    Uuid uuid = Uuid::GenerateRandom();

    EXPECT_EQ(Uuid::FromGuid(uuid.toGuid()), uuid);
    EXPECT_EQ(Uuid::toString(uuid.toGuid()), "{" + uuid.toString("-") + "}");
    EXPECT_EQ(Uuid::toString(uuid.toGuid()), uuid.toWindowsString());
}

TEST(UUID, from)
{
    Uuid uuid = Uuid::GenerateRandom();
    auto ret  = Uuid::FromWindowsString(uuid.toWindowsString());
    ASSERT_TRUE(ret.has_value());
    EXPECT_EQ(*ret, uuid);
    EXPECT_EQ(Uuid::toString(uuid.toGuid()), uuid.toWindowsString());
    EXPECT_EQ(Uuid::toString(ret->toGuid()), uuid.toWindowsString());
}
#endif

TEST(UUID, generate)
{
    Uuid uuid = Uuid::GenerateRandom();
    EXPECT_EQ(32, uuid.toString("").size());
}

TEST(Crypt, Digest)
{
    const std::string kDigit      = "0123456789";
    const std::string kAlpha      = "abcdefghijklmnopqrstuvwxyz";
    const std::string kAlphaDigit = R"(0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz)";
    const std::string kASCII      = R"(!"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)";
    const std::string kLongASCII  = zeus::Repeat(kASCII, 100);

    const std::string kDigitMd5      = "781e5e245d69b566979b86e28d23f2c7";
    const std::string kAlphaMd5      = "c3fcd3d76192e4007dfb496cca67e13b";
    const std::string kAlphaDigitMd5 = "2b9f53830380dbc745afe8ccd814d710";
    const std::string kLongASCIIMd5  = "df90e02477b39429b2c2e5289b2a47aa";

    const std::string kDigitSHA1      = "87acec17cd9dcd20a716cc2cf67417b71c8a7016";
    const std::string kAlphaSHA1      = "32d10c7b8cf96570ca04ce37f2a19d84240d3a89";
    const std::string kAlphaDigitSHA1 = "3cbdaa42769c27d4fb397baef559653aad1af134";
    const std::string kLongASCIISHA1  = "84844480a563748e55a6d14305d3dd8087c8063d";

    const std::string kDigitSHA224      = "f28ad8ecd48ba6f914c114821685ad08f0d6103649ff156599a90426";
    const std::string kAlphaSHA224      = "45a5f72c39c5cff2522eb3429799e49e5f44b356ef926bcf390dccc2";
    const std::string kAlphaDigitSHA224 = "c6d99ea870fe752ab5eab79fe809535668f60cfb32e9a022c2099212";
    const std::string kLongASCIISHA224  = "02be2d64682f9a9eca435f8fe4a7d6aa7f6c20cf6c226afbf4710341";

    const std::string kDigitSHA256      = "84d89877f0d4041efb6bf91a16f0248f2fd573e6af05c19f96bedb9f882f7882";
    const std::string kAlphaSHA256      = "71c480df93d6ae2f1efad1447c66c9525e316218cf51fc8d9ed832f2daf18b73";
    const std::string kAlphaDigitSHA256 = "36e0749b5c36886c70524e5eb6de0b2c289abba2062a953b7a3c75beab5d49be";
    const std::string kLongASCIISHA256  = "d710584b9a67a7a14b9c4e5f1a941cd50c4aae93e5c57812c63c56cc300a3aea";

    const std::string kDigitSHA384      = "90ae531f24e48697904a4d0286f354c50a350ebb6c2b9efcb22f71c96ceaeffc11c6095e9ca0df0ec30bf685dcf2e5e5";
    const std::string kAlphaSHA384      = "feb67349df3db6f5924815d6c3dc133f091809213731fe5c7b5f4999e463479ff2877f5f2936fa63bb43784b12f3ebb4";
    const std::string kAlphaDigitSHA384 = "810ef9e4256e044ce5eb9992bfb59d468bfb57de82ee734bfa1d3115b83fcfb7fe7670160ba475bc81ba2d4f9e75a772";
    const std::string kLongASCIISHA384  = "05571cf1107f983c115ab90a1ac98be336d483e62323a19340b070cfdd0915f725e58bd8e741bbe0d5b918300faf08e5";

    const std::string kDigitSHA512 =
        "bb96c2fc40d2d54617d6f276febe571f623a8dadf0b734855299b0e107fda32cf6b69f2da32b36445d73690b93cbd0f7bfc20e0f7f28553d2a4428f23b716e90";
    const std::string kAlphaSHA512 =
        "4dbff86cc2ca1bae1e16468a05cb9881c97f1753bce3619034898faa1aabe429955a1bf8ec483d7421fe3c1646613a59ed5441fb0f321389f77f48a879c7b1f1";
    const std::string kAlphaDigitSHA512 =
        "ea9d39795bd0c9287aba9b8f48c78f8334e4ec6505d2b268e11caaa9bbd74fd5538cca53a2b1bacd006f71d6052c800edec9b1a57973912b59f0e8c475f9a94d";
    const std::string kLongASCIISHA512 =
        "701e84a1e3285c88294d676a45e1f43482372445047217098be772399d74409f097d0f9aa82254c49a5a5e0394d2368bc57a2258808ed2c2de3ec87e84588e0e";

    const std::string kDigitCrc32      = "c6c784a6";
    const std::string kAlphaCrc32      = "bd50274c";
    const std::string kAlphaDigitCrc32 = "05e2f0a0";
    const std::string kLongASCIICrc32  = "22e6942d";

    const std::string kDigitCrc64Ecma182      = "1e73127f2ccf6527";
    const std::string kAlphaCrc64Ecma182      = "2f121b7575789626";
    const std::string kAlphaDigitCrc64Ecma182 = "9901a059e6c40c8a";
    const std::string kLongASCIICrc64Ecma182  = "e8d30fe650cb893a";

    const std::string kHMACSalt = "zeusZeusZEUSzEuszEUszEUZS";

    const std::string kDigitHMACMD5      = "57459d999556ec44a2c94a08f4f6627a";
    const std::string kAlphaHMACMD5      = "5c9d654c169916acc46aa659ae765a67";
    const std::string kAlphaDigitHMACMD5 = "d96154a2306affca848af805c93acdbb";
    const std::string kLongASCIIHMACMD5  = "b9e97e9c039caaede299bc6039f9d848";

    const std::string kDigitHMACSHA1      = "cf4b553ed8d8a3533332afb0b77a0df5b492990a";
    const std::string kAlphaHMACSHA1      = "323d0c7af9fed171e5eb8b0a7af3033a78262e40";
    const std::string kAlphaDigitHMACSHA1 = "aa023dffe6807d8e14fd5f6eccb17bf58e4f10a4";
    const std::string kLongASCIIHMACSHA1  = "79eca386e341de4f41a911b941b04844da3bdde6";

    const std::string kDigitHMACSHA224      = "bbcfd23c869751e8416269a8f77e6ba97daa18bc2661e95aea54ede4";
    const std::string kAlphaHMACSHA224      = "19d1b6917fb6bc9b071e4cb404a49758a2365c74d14d0ec4b974b664";
    const std::string kAlphaDigitHMACSHA224 = "42571207e211f576e1cbfe88ef7ee71ac89a9093cef010223f915b92";
    const std::string kLongASCIIHMACSHA224  = "79b9fb893fbaa10fcc5985be270ca99cb1ddbf88f14bb40f9de60191";

    const std::string kDigitHMACSHA256      = "6c39bb3e12548b22b9aecbc2e49cf2c9f8b9b55c46c6eb3351c6822aa4de7c0c";
    const std::string kAlphaHMACSHA256      = "7842b4e420a241122c2c5ad12c3c35be7efb1bb6bd81f1ed367b5cb048c6f48c";
    const std::string kAlphaDigitHMACSHA256 = "6d5693056d1eb910a3fa4ec0ce7d5c61512f54c203939924fffa0b4d2f5ac5fe";
    const std::string kLongASCIIHMACSHA256  = "79227dd0f8bfc91da54b0611bbfc2ed7072ae90f10694e636a4d22b60204f469";

    const std::string kDigitHMACSHA384      = "64c715ac8bf02629f459b70313342a5083e077d4f029e292c22e9fbcfb68c62b6b2230f94535d2a1c4daf03359f42b17";
    const std::string kAlphaHMACSHA384      = "21196f7c7ef0d1b2333500fa8e2168d9e86cfed3747a342ec489a51477c1f9c80280846d58ad2b76d110ee2f8e8d94e3";
    const std::string kAlphaDigitHMACSHA384 = "f01f8c8d0126835038594084983d5f59f4142d527021ca4f32936c4f75f2a7f3255a7c224bae3c53ca505d5f29f363f5";
    const std::string kLongASCIIHMACSHA384  = "4164b457310a476e5ee7a545c721cb49b530eef5ef367355ec43ac69f56dae8b458553a4335fb124bef731ad31f9c930";

    const std::string kDigitHMACSHA512 =
        "50547552cd3e0caa43422e5218739730ce78dd493b179e7e0f869d760491b9dc2c43fcd3d3ef9eaaa1b8f8270e2cd1a04b60e7bbab8626a2c1d9e072debf5e13";
    const std::string kAlphaHMACSHA512 =
        "6ee86998c411a3d1589053482aba79e2bfb3e0270dfd7e202654cb66a5e0648b92ac2e1d0624a9285ac125dd3373a3eced1c902201c110a1027748d99477a2aa";
    const std::string kAlphaDigitHMACSHA512 =
        "0d475f99457a62d3eae63425387a22e31cf1cdea737e5319645accea9d690c28d674bf1cf90e769dd008d66284d9b5a2668cb41d47a1c80fda2c721900f64694";
    const std::string kLongASCIIHMACSHA512 =
        "a78c1f7f895f13367fe64ec58d54222da766792e0f00135132fdaac48cb3ce181e3e1b54bb466d7d8274c1a804591f9bd729d18d47177a6d2ed8a64ae3bcf44d";

#define TEST_DIGEST(digestName)                                   \
    {                                                             \
        digestName##Digest digest;                                \
        TestDigest(digest, kDigit, kDigit##digestName);           \
        TestDigest(digest, kAlpha, kAlpha##digestName);           \
        TestDigest(digest, kAlphaDigit, kAlphaDigit##digestName); \
        TestDigest(digest, kLongASCII, kLongASCII##digestName);   \
    }

#define TEST_HMAC_DIGEST(digestName)                                                       \
    {                                                                                      \
        HMACDigest digest(kHMACSalt.data(), kHMACSalt.size(), HMACDigestType::digestName); \
        TestDigest(digest, kDigit, kDigitHMAC##digestName);                                \
        TestDigest(digest, kAlpha, kAlphaHMAC##digestName);                                \
        TestDigest(digest, kAlphaDigit, kAlphaDigitHMAC##digestName);                      \
        TestDigest(digest, kLongASCII, kLongASCIIHMAC##digestName);                        \
    }

    TEST_DIGEST(Md5);
    TEST_DIGEST(SHA1);
    TEST_DIGEST(SHA224);
    TEST_DIGEST(SHA256);
    TEST_DIGEST(SHA384);
    TEST_DIGEST(SHA512);
    TEST_DIGEST(Crc32);
    TEST_DIGEST(Crc64Ecma182);

    TEST_HMAC_DIGEST(MD5);
    TEST_HMAC_DIGEST(SHA1);
    TEST_HMAC_DIGEST(SHA224);
    TEST_HMAC_DIGEST(SHA256);
    TEST_HMAC_DIGEST(SHA384);
    TEST_HMAC_DIGEST(SHA512);
}

static void DoCryptoGTest(BaseEncrypt &encrypt, BaseDecrypt &decrypt, const string &expectPlanText, const string &expectCipherText)
{
    {
        encrypt.Update(expectPlanText);
        string cipherText = encrypt.GetString();

        ASSERT_EQ(expectCipherText, cipherText);
        ASSERT_EQ(expectCipherText.size(), encrypt.GetSize());

        decrypt.Update(expectCipherText);
        string plainText = decrypt.GetString();

        EXPECT_EQ(expectPlanText, plainText);
        EXPECT_EQ(expectPlanText.size(), decrypt.GetSize());

        encrypt.Reset();
        decrypt.Reset();
    }
    {
        encrypt.Update(expectPlanText.data(), expectPlanText.size());
        string cipherText = encrypt.GetString();

        ASSERT_EQ(expectCipherText, cipherText);
        ASSERT_EQ(expectCipherText.size(), encrypt.GetSize());

        decrypt.Update(cipherText.c_str(), cipherText.size());
        string plainText = decrypt.GetString();

        EXPECT_EQ(expectPlanText, plainText);
        EXPECT_EQ(expectPlanText.size(), decrypt.GetSize());

        encrypt.Reset();
        decrypt.Reset();
    }
    {
        stringstream planTextStream(expectPlanText);

        while (!planTextStream.eof())
        {
            static const unsigned int BUFF_SIZE = 1024;
            char                      buff[BUFF_SIZE];
            planTextStream.read(buff, BUFF_SIZE);
            if (streamsize n = planTextStream.gcount())
            {
                encrypt.Update(buff, n);
            }
        }
        string cipherText = encrypt.GetString();

        ASSERT_EQ(expectCipherText, cipherText);
        ASSERT_EQ(expectCipherText.size(), encrypt.GetSize());

        stringstream cipherTextStream(expectCipherText);
        while (!cipherTextStream.eof())
        {
            static const unsigned int BUFF_SIZE = 1024;
            char                      buff[BUFF_SIZE];
            cipherTextStream.read(buff, BUFF_SIZE);
            if (streamsize n = cipherTextStream.gcount())
            {
                decrypt.Update(buff, n);
            }
        }

        string plainText = decrypt.GetString();

        EXPECT_EQ(expectPlanText, plainText);
        EXPECT_EQ(expectPlanText.size(), decrypt.GetSize());
        encrypt.Reset();
        decrypt.Reset();
    }
    {
        auto planTextFilePath = fs::temp_directory_path() / RandWord();
        ofstream(planTextFilePath, std::ios::binary).write(expectPlanText.data(), expectPlanText.size());
        auto cipherTextFilePath = fs::temp_directory_path() / RandWord();
        ofstream(cipherTextFilePath, std::ios::binary).write(expectCipherText.data(), expectCipherText.size());
        encrypt.Update(planTextFilePath, true);
        string cipherText = encrypt.GetString();

        ASSERT_EQ(expectCipherText, cipherText);
        ASSERT_EQ(expectCipherText.size(), encrypt.GetSize());

        decrypt.Update(cipherTextFilePath, true);
        string plainText = decrypt.GetString();

        EXPECT_EQ(expectPlanText, plainText);
        EXPECT_EQ(expectPlanText.size(), decrypt.GetSize());

        encrypt.Reset();
        decrypt.Reset();
    }
}

TEST(Base64, digit)
{
    string planText("0123456789");

    string expectCipherText    = "MDEyMzQ1Njc4OQ==";
    string expectCipherUrlText = "MDEyMzQ1Njc4OQ";

    Base64Encrypt encrypt(false);
    Base64Decrypt decrypt(false);

    DoCryptoGTest(encrypt, decrypt, planText, expectCipherText);

    Base64Encrypt encryptUrl(true);
    Base64Decrypt decryptUrl(true);

    DoCryptoGTest(encryptUrl, decryptUrl, planText, expectCipherUrlText);
}

TEST(Base64, alpha)
{
    string planText("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

    string expectCipherText    = "QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVphYmNkZWZnaGlqa2xtbm9wcXJzdHV2d3h5eg==";
    string expectCipherUrlText = "QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVphYmNkZWZnaGlqa2xtbm9wcXJzdHV2d3h5eg";

    Base64Encrypt encrypt(false);
    Base64Decrypt decrypt(false);

    DoCryptoGTest(encrypt, decrypt, planText, expectCipherText);

    Base64Encrypt encryptUrl(true);
    Base64Decrypt decryptUrl(true);

    DoCryptoGTest(encryptUrl, decryptUrl, planText, expectCipherUrlText);
}

TEST(Base64, printchar)
{
    string planText(R"( !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)");

    string expectCipherText =
        "ICEiIyQlJicoKSorLC0uLzAxMjM0NTY3ODk6Ozw9Pj9AQUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVpbXF1eX2BhYmNkZWZnaGlqa2xtbm9wcXJzdHV2d3h5ent8fX4=";
    string expectCipherUrlText =
        "ICEiIyQlJicoKSorLC0uLzAxMjM0NTY3ODk6Ozw9Pj9AQUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVpbXF1eX2BhYmNkZWZnaGlqa2xtbm9wcXJzdHV2d3h5ent8fX4";

    Base64Encrypt encrypt(false);
    Base64Decrypt decrypt(false);

    DoCryptoGTest(encrypt, decrypt, planText, expectCipherText);

    Base64Encrypt encryptUrl(true);
    Base64Decrypt decryptUrl(true);

    DoCryptoGTest(encryptUrl, decryptUrl, planText, expectCipherUrlText);
}

TEST(Base64, urlchar)
{
    string planText = "i\xb7\x1d\xfb\xef\xff";

    string expectCipherText    = "abcd++//";
    string expectCipherUrlText = "abcd--__";

    Base64Encrypt encrypt(false);
    Base64Decrypt decrypt(false);

    DoCryptoGTest(encrypt, decrypt, planText, expectCipherText);

    Base64Encrypt encryptUrl(true);
    Base64Decrypt decryptUrl(true);

    DoCryptoGTest(encryptUrl, decryptUrl, planText, expectCipherUrlText);
}

TEST(Base64, longstring_three)
{
    string planText = string(BASE64_LONGSTRING_THREE, sizeof(BASE64_LONGSTRING_THREE) - 1);

    string expectCipherText    = string(BASE64_LONGSTRING_THREE_CIPHERTEXT, sizeof(BASE64_LONGSTRING_THREE_CIPHERTEXT) - 1);
    string expectCipherUrlText = string(BASE64_LONGSTRING_THREE_CIPHERTEXTURL, sizeof(BASE64_LONGSTRING_THREE_CIPHERTEXTURL) - 1);

    Base64Encrypt encrypt(false);
    Base64Decrypt decrypt(false);

    DoCryptoGTest(encrypt, decrypt, planText, expectCipherText);

    Base64Encrypt encryptUrl(true);
    Base64Decrypt decryptUrl(true);

    DoCryptoGTest(encryptUrl, decryptUrl, planText, expectCipherUrlText);
}

TEST(Base64, longstring_not_three)
{
    string planText = string(BASE64_LONGSTRING_NOT_THREE, sizeof(BASE64_LONGSTRING_NOT_THREE) - 1);

    string expectCipherText    = string(BASE64_LONGSTRING_NOT_THREE_CIPHERTEXT, sizeof(BASE64_LONGSTRING_NOT_THREE_CIPHERTEXT) - 1);
    string expectCipherUrlText = string(BASE64_LONGSTRING_NOT_THREE_CIPHERTEXTURL, sizeof(BASE64_LONGSTRING_NOT_THREE_CIPHERTEXTURL) - 1);

    Base64Encrypt encrypt(false);
    Base64Decrypt decrypt(false);

    DoCryptoGTest(encrypt, decrypt, planText, expectCipherText);

    Base64Encrypt encryptUrl(true);
    Base64Decrypt decryptUrl(true);

    DoCryptoGTest(encryptUrl, decryptUrl, planText, expectCipherUrlText);
}

TEST(Base64, BigLong)
{
    string planText            = string(BASE64_LONGSTRING_THREE, sizeof(BASE64_LONGSTRING_THREE) - 1);
    planText                   = Repeat(planText, 2048);
    string expectCipherText    = string(BASE64_LONGSTRING_THREE_CIPHERTEXT, sizeof(BASE64_LONGSTRING_THREE_CIPHERTEXT) - 1);
    expectCipherText           = Repeat(expectCipherText, 2048);
    string expectCipherUrlText = string(BASE64_LONGSTRING_THREE_CIPHERTEXTURL, sizeof(BASE64_LONGSTRING_THREE_CIPHERTEXTURL) - 1);
    expectCipherUrlText        = Repeat(expectCipherUrlText, 2048);

    Base64Encrypt encrypt(false);
    Base64Decrypt decrypt(false);

    DoCryptoGTest(encrypt, decrypt, planText, expectCipherText);

    Base64Encrypt encryptUrl(true);
    Base64Decrypt decryptUrl(true);

    DoCryptoGTest(encryptUrl, decryptUrl, planText, expectCipherUrlText);
}

TEST(AES, base)
{
    const std::string kASCII   = R"(!"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)";
    string            planText = Repeat(kASCII, 10);

    string key = "9x38fy2138ebda5)da2am3ea008d86z9";
    string iv  = "(c86747b7460c61z";
    string cipherText_CFB_PKCS7_BASE64 =
        R"(EV+kgMV5PlIQhhbGHgeJqzn4bkSLZ+jz485m67fdHdEdewsS8SnX/d+TVXkT4tjIhTWIrTYabEHHUTFAhgAg6nJeEy8B8HzmtcKkGkjoXS4RwFhoVxmoI5sLwNQVKG4nHMjO7MWdFFSxybRwCiOKY/STYnL7Rj2ZsKIA0RKlsWJK4G4LaeYEuIcbGpoHdPK1e1en3DhAXdap2liEq6iQwUI+B5zoIi1tx+eUPrt5kPRHnE1kA2g2NeQIJD7ZJXMITscunxHdZPqn7oPhBWnRg+3RkLkyorhotOp5qd7jEE1k54zRhar3kuUO/6w4OnEWhV8fdWWyMM30oXzMTJPchA9cU4mxD3EOd9SeXRhgj3gF5+SnhWQL7ak5LPxcakQ9R+m5bIXFvJa0H3K11nSx6nPJZ5DNYOl2knEtXg8mC2DHQPgZSAnC68KRVYDZbHEj8R4r4WSSOFjsrF7LVKmSq9T5dz5p1Fa/EtlQ27jxKGdXnrS9E3IQVuXkbokJsHv/F/68zM+uHyFxHXoVPUbvctiB2lw0/JOLLdrlDPZ7t+Bh1w7z82Sv+QkllvT5146U+O2C5G2EsUYL9C6RZLj6Ps7nVd1/YAiv3mPMbZEgbPZuymeCrB48wY7TFplRwrmrHoekLJY9jAfA1Bm+fxYhflXu/rbxOqIrMBvOyhOrYKfL9um5g76p7FckZ+qiRA8s2S1nFZlla1DyUkyJ/m1buBsCn7+KAf41en4Dh8Hg9rthStr3wuaWZgBN6Te9WAWh/cEi2rm46f06anP4WmQzNeuYp/ZkG2+FnKbcfrc9ZdyXdrWM6BskefLcTpR2gKEBw9p3Q2VgdKiMLGKK11ZI/DQ5rcEqiZfYYvBGIDUchQeDFV74lF/lKKpJG2COCgD+C8ImFrk/rf+ZtdB9gB0zZTnRC75YF8uP78j+f7yt1HPh8g7GmjXh/VZ8FHVt2YOiH5M5bohyWsBPKzbvUCswNem6QSEq0dZk8BO3Jw319hYO6LiuIfjuzaWf65ag4rVS2ALxhSwEudlxBWs/oUk/Ivho6Ekq6/hhKJtuDZU13Ou/Q/R6tis2QXht1OXgc6huuYgEr3OT1JHPvigqzTqY7yU51jTzCJZ0e7rx8du5F5fRZk7CwgRssqtOnw9+GzeQ10vcDfENy02VvRK9SebNxtaO0aPGotuZDRek3R8Tyr+0Qh3Zj53tvFhfTAfavOHn6lbBaFbkNwR11oAgd9EuiRImfmDaOkimV0b4cSkqRKM=)";
    string cipherText_CFB_ISO10126_BASE64 =
        R"(EV+kgMV5PlIQhhbGHgeJqzn4bkSLZ+jz485m67fdHdEdewsS8SnX/d+TVXkT4tjIhTWIrTYabEHHUTFAhgAg6nJeEy8B8HzmtcKkGkjoXS4RwFhoVxmoI5sLwNQVKG4nHMjO7MWdFFSxybRwCiOKY/STYnL7Rj2ZsKIA0RKlsWJK4G4LaeYEuIcbGpoHdPK1e1en3DhAXdap2liEq6iQwUI+B5zoIi1tx+eUPrt5kPRHnE1kA2g2NeQIJD7ZJXMITscunxHdZPqn7oPhBWnRg+3RkLkyorhotOp5qd7jEE1k54zRhar3kuUO/6w4OnEWhV8fdWWyMM30oXzMTJPchA9cU4mxD3EOd9SeXRhgj3gF5+SnhWQL7ak5LPxcakQ9R+m5bIXFvJa0H3K11nSx6nPJZ5DNYOl2knEtXg8mC2DHQPgZSAnC68KRVYDZbHEj8R4r4WSSOFjsrF7LVKmSq9T5dz5p1Fa/EtlQ27jxKGdXnrS9E3IQVuXkbokJsHv/F/68zM+uHyFxHXoVPUbvctiB2lw0/JOLLdrlDPZ7t+Bh1w7z82Sv+QkllvT5146U+O2C5G2EsUYL9C6RZLj6Ps7nVd1/YAiv3mPMbZEgbPZuymeCrB48wY7TFplRwrmrHoekLJY9jAfA1Bm+fxYhflXu/rbxOqIrMBvOyhOrYKfL9um5g76p7FckZ+qiRA8s2S1nFZlla1DyUkyJ/m1buBsCn7+KAf41en4Dh8Hg9rthStr3wuaWZgBN6Te9WAWh/cEi2rm46f06anP4WmQzNeuYp/ZkG2+FnKbcfrc9ZdyXdrWM6BskefLcTpR2gKEBw9p3Q2VgdKiMLGKK11ZI/DQ5rcEqiZfYYvBGIDUchQeDFV74lF/lKKpJG2COCgD+C8ImFrk/rf+ZtdB9gB0zZTnRC75YF8uP78j+f7yt1HPh8g7GmjXh/VZ8FHVt2YOiH5M5bohyWsBPKzbvUCswNem6QSEq0dZk8BO3Jw319hYO6LiuIfjuzaWf65ag4rVS2ALxhSwEudlxBWs/oUk/Ivho6Ekq6/hhKJtuDZU13Ou/Q/R6tis2QXht1OXgc6huuYgEr3OT1JHPvigqzTqY7yU51jTzCJZ0e7rx8du5F5fRZk7CwgRssqtOnw9+GzeQ10vcDfENy02VvRK9SebNxtaO0aPGotuZDRek3R8Tyr+0Qh3Zj53tvFhfTAfavOHn6lbBaFbkNwR11oAgd9EuiRImfmDaOkimV0b4cbt2WKM=)";
    string cipherText_CFB_ANSIX923_BASE64 =
        R"(EV+kgMV5PlIQhhbGHgeJqzn4bkSLZ+jz485m67fdHdEdewsS8SnX/d+TVXkT4tjIhTWIrTYabEHHUTFAhgAg6nJeEy8B8HzmtcKkGkjoXS4RwFhoVxmoI5sLwNQVKG4nHMjO7MWdFFSxybRwCiOKY/STYnL7Rj2ZsKIA0RKlsWJK4G4LaeYEuIcbGpoHdPK1e1en3DhAXdap2liEq6iQwUI+B5zoIi1tx+eUPrt5kPRHnE1kA2g2NeQIJD7ZJXMITscunxHdZPqn7oPhBWnRg+3RkLkyorhotOp5qd7jEE1k54zRhar3kuUO/6w4OnEWhV8fdWWyMM30oXzMTJPchA9cU4mxD3EOd9SeXRhgj3gF5+SnhWQL7ak5LPxcakQ9R+m5bIXFvJa0H3K11nSx6nPJZ5DNYOl2knEtXg8mC2DHQPgZSAnC68KRVYDZbHEj8R4r4WSSOFjsrF7LVKmSq9T5dz5p1Fa/EtlQ27jxKGdXnrS9E3IQVuXkbokJsHv/F/68zM+uHyFxHXoVPUbvctiB2lw0/JOLLdrlDPZ7t+Bh1w7z82Sv+QkllvT5146U+O2C5G2EsUYL9C6RZLj6Ps7nVd1/YAiv3mPMbZEgbPZuymeCrB48wY7TFplRwrmrHoekLJY9jAfA1Bm+fxYhflXu/rbxOqIrMBvOyhOrYKfL9um5g76p7FckZ+qiRA8s2S1nFZlla1DyUkyJ/m1buBsCn7+KAf41en4Dh8Hg9rthStr3wuaWZgBN6Te9WAWh/cEi2rm46f06anP4WmQzNeuYp/ZkG2+FnKbcfrc9ZdyXdrWM6BskefLcTpR2gKEBw9p3Q2VgdKiMLGKK11ZI/DQ5rcEqiZfYYvBGIDUchQeDFV74lF/lKKpJG2COCgD+C8ImFrk/rf+ZtdB9gB0zZTnRC75YF8uP78j+f7yt1HPh8g7GmjXh/VZ8FHVt2YOiH5M5bohyWsBPKzbvUCswNem6QSEq0dZk8BO3Jw319hYO6LiuIfjuzaWf65ag4rVS2ALxhSwEudlxBWs/oUk/Ivho6Ekq6/hhKJtuDZU13Ou/Q/R6tis2QXht1OXgc6huuYgEr3OT1JHPvigqzTqY7yU51jTzCJZ0e7rx8du5F5fRZk7CwgRssqtOnw9+GzeQ10vcDfENy02VvRK9SebNxtaO0aPGotuZDRek3R8Tyr+0Qh3Zj53tvFhfTAfavOHn6lbBaFbkNwR11oAgd9EuiRImfmDaOkimV0b4cS0uQKM=)";
    string cipherText_ECB_PKCS7_BASE64 =
        R"(PBPo53xK86h8al4yE0iPesLzv3j6T/qV5xJKsfN52nNU6M5ualvrdCwTjwsgUqIzUq/79QcJ0FlNttt4+etEPqz6kupEXuOoYDqdtbk7o2NXVzwAaFliQ9d5Qk/JAnzJ5nljrC9l71T8MhTx2zXd6f5ELuPtMpCCO16olBhENsbUkLTfLHYyPrt+iniUBRmG4sTik5TX04eAsYjbuR5oWkQtKVPwviPwI6L+yVN8FwVyv9PHyrkxBSNReH0tshKqs6wg3ujGHJq/CZnhQlml6MvPefgn17ShB69N3EB8fKeg66ILT5z5fbICIF7SaN7Iqc7DAR10cM/92EefWfRNkRdjEWKkirjJsIdPhaSEeJgOiGaInXHBqYcSAiqTYWTzr4+PSmZkMYVr+VNFZVf25gzi7BYsvYG4pKf2rEUviX7FEeoASHg15nGojF3Onfeu73pk1B634wGfZ+O25G68N5Qp/+2hgzkcXU0loE+9/lYcNubkxbj+t/m4zx6Z7hXH+h99FcVRvoS1nYK9/qJndSozb0WHGoIthpfOfb75I2RzQXow8akkbKfutKHDtVNDvmyxlPMvzuzZwawKZXKB2hwdWgu6SeW1HdI8jtrNp/5YaOJVPomvlpnxfurZsTcN7qg7HJxu9zw+MhkMiTMTQV8rXbpODZjam0M94POsvgEKMrdGm2LFDM4xqHWBpJ86GMM2X+mLCTth93BvaT1WBmGCkqbjjbt19yt39nsi+Qg9s67C70QXcq9iaJOAzCgzZd/VVr4kmvjWEKqrgOA8I/+fOUsCj+U6DJi9PxvNJ6LlLGGzpAWnceyNYOpSVCFLJPMePR3cEfQ5qcomwPUe49BXAnMxzKd4SE7Dhw50YftpMioOEGA5/OeG92KiyxiDBmv/L+9pK5kMbgu5zIrIUmJD/zvdG3Iiz+8Yr2DQz5oMEGYVO9WZ+615iDMsUeqdACIMwxBt/c2nm+TiMWPt855XJEY8FimaDHbQCAlXfzY8E+jnfErzqHxqXjITSI96wvO/ePpP+pXnEkqx83nac1Tozm5qW+t0LBOPCyBSojNSr/v1BwnQWU2223j560Q+rPqS6kRe46hgOp21uTujY1dXPABoWWJD13lCT8kCfMnmeWOsL2XvVPwyFPHbNd3p/kQu4+0ykII7XqiUGEQ2xtSQtN8sdjI+u36KeJQFGYbixOKTlNfTh4CxiNu5HmhaRC0pU/C+I/Ajov7JU3wXBWpn16o9mblSU3+FrbCJ95A=)";

    string cipherText_ECB_ANSIX923_BASE64 =
        R"(PBPo53xK86h8al4yE0iPesLzv3j6T/qV5xJKsfN52nNU6M5ualvrdCwTjwsgUqIzUq/79QcJ0FlNttt4+etEPqz6kupEXuOoYDqdtbk7o2NXVzwAaFliQ9d5Qk/JAnzJ5nljrC9l71T8MhTx2zXd6f5ELuPtMpCCO16olBhENsbUkLTfLHYyPrt+iniUBRmG4sTik5TX04eAsYjbuR5oWkQtKVPwviPwI6L+yVN8FwVyv9PHyrkxBSNReH0tshKqs6wg3ujGHJq/CZnhQlml6MvPefgn17ShB69N3EB8fKeg66ILT5z5fbICIF7SaN7Iqc7DAR10cM/92EefWfRNkRdjEWKkirjJsIdPhaSEeJgOiGaInXHBqYcSAiqTYWTzr4+PSmZkMYVr+VNFZVf25gzi7BYsvYG4pKf2rEUviX7FEeoASHg15nGojF3Onfeu73pk1B634wGfZ+O25G68N5Qp/+2hgzkcXU0loE+9/lYcNubkxbj+t/m4zx6Z7hXH+h99FcVRvoS1nYK9/qJndSozb0WHGoIthpfOfb75I2RzQXow8akkbKfutKHDtVNDvmyxlPMvzuzZwawKZXKB2hwdWgu6SeW1HdI8jtrNp/5YaOJVPomvlpnxfurZsTcN7qg7HJxu9zw+MhkMiTMTQV8rXbpODZjam0M94POsvgEKMrdGm2LFDM4xqHWBpJ86GMM2X+mLCTth93BvaT1WBmGCkqbjjbt19yt39nsi+Qg9s67C70QXcq9iaJOAzCgzZd/VVr4kmvjWEKqrgOA8I/+fOUsCj+U6DJi9PxvNJ6LlLGGzpAWnceyNYOpSVCFLJPMePR3cEfQ5qcomwPUe49BXAnMxzKd4SE7Dhw50YftpMioOEGA5/OeG92KiyxiDBmv/L+9pK5kMbgu5zIrIUmJD/zvdG3Iiz+8Yr2DQz5oMEGYVO9WZ+615iDMsUeqdACIMwxBt/c2nm+TiMWPt855XJEY8FimaDHbQCAlXfzY8E+jnfErzqHxqXjITSI96wvO/ePpP+pXnEkqx83nac1Tozm5qW+t0LBOPCyBSojNSr/v1BwnQWU2223j560Q+rPqS6kRe46hgOp21uTujY1dXPABoWWJD13lCT8kCfMnmeWOsL2XvVPwyFPHbNd3p/kQu4+0ykII7XqiUGEQ2xtSQtN8sdjI+u36KeJQFGYbixOKTlNfTh4CxiNu5HmhaRC0pU/C+I/Ajov7JU3wXBUgGtZhlIBvoFIWLzdlp5Yo=)";
    string cipherText_CBC_PKCS7_BASE64 =
        R"(BSHXGVfA2icuUEHUNPsj+L12qTrlfjXO/6+6vsDzfejGf9L485T7KNG4oe+qJUm/M2MuQPlUcoddt2Rogw5TIY899JyObC1qTm8o6isAb86PRJ2wHGcjBBXhW4yRUswEFeRqdF9N8jqk84g3rJuJ3x4uFCD/lsy72L+S75Utw8jT3V4riXx1ytDDq/Nh3Yp6r8M6Ee9/Lfb/5zoeVO3TkAYThqA2JiTtEI0pQ1JJiAg4KxkixoLENvWubAai0b5PSZaJX9NfQPJSPi5KK2lE+HvMeYUlqzgg1hyJ/CRfGEflO9xUoXRWaypxT154+1paPDh6eImiOHzmQAVQSyTuEwoNmFyoE6I43xJR9jgeHgreVyPIz4WZJVRDztdD7ie2TZMulxmhWhDuv62LdjjnvM8Km0jZU0B79vPt7Sbk0Khx3gYL6ouFuEyssiUAjpUgJTPyDIRuAW9Z9ckLKTH2Cv7FRQ84SGmjjHhc+VCHSRzpz7u7QWRTgCyrffdSKno9QhSuhB3kMkIgxtyFeYR+MSqkiOAmG5AhNEWsVtoH/BjlT5geity+WNTJ9Hs6SaZwJMxAMu2JJVuzo9ywVuMvyfeJoybhkFrn/5uDyPY7cNGynahDnK/Bbc5jn4Xxt45vGHXeL6IYsFY2pjgZS8svtdgJWtTL/zytpM0C0Ly3QKqFZtvtOsULtirozm5xAT2zwiPek6LcCqpCyFxgXfXIU1eVFfaxb4UjQ9VQr+aLQBqTW7Oz2EbUHOX8xULSbo5zb5U0zwRjzPrfATLE0DYAgGQXUjlXAwThdktZm9xMO6Lb9G11ohhXxFDK6PUT77aId7WZdRlFB3JXK+rwcJc5v9mSzabgS18ZJNnPwlDQlTpY/uhl2nmjqm+rEYhNO2ZqEPY5lxRJIqYMwaXMReLVRVA/neDxPddzU48RNAhDRnaoVYk29JTFyXKyD0BHrFC4CN1MASlyOOOQrfGgH45pOR2MfsQA3JstYxCxlABDGL3hEtQDLQYt3HDqD71Fc9ZbGRNs+jMRusEoP7L2UjQXXUlMFh5BlPZUcJ53/11AZmp6C6yYLv0JDADIrUm/NplKIVQJcd5LDVAbrxlBJXFUoNi3FQXCUxcmAPWZCX9/1GdMfEeEXQbSBkgFG7qblTRmgDBpf/cCVtN6BtmGHOWkSoyfi51PECnfsQOUQHfoltc2FRvjxF6lz7uAhS7Mra1I3WWZhj/R0zY6IdqabDDj80J6Dd7ZVt6m4WKINySQY78=)";

    string cipherText_CTR_PKCS7_BASE64 =
        R"(EV+kgMV5PlIQhhbGHgeJq+/xlgHZnj7m3KHhbBu4zbU7DjVvr9iJKH3bGy5U/OV0p9P+OcDmpFClyr1Q3MTwlM+QLfeQMGn2ekF8ZHfDDQdvyCUadJSdzoJ+FHJHr40y0lqyl0TanuZpAfkA5Q9sZsOmB9gOKs9KhvfSCndUcRHFg9ZDputSTt49mfSZR87NUcdjduuS6znVOrXmtou8rLDEGZZXWVUZHKVRLDGU9wktljJ4NPe1vtpvTjjiM8ev6qGTcHnLuq82O23K4ldXEXEmasAh9X9YtumJnAyMe+oVvtVBD7dJw/vUOaCIFZD38lZ7gIlnI0Uqum6i7JmxjlJCDdT4VLvvhyFNN6sO3VigXAw33mO02QIppWbUQAgN5ZHYjCRfcTv2SRb4aENPn+bUBIn2itE8qfnpSo37sfuPKququrJhG7C0oGJNpHHLx4itLmTgmKccslBL4P9mnHSX5CfhQfAhIynCt7r58yrWdKwCBXz0jSFjd6r4c3DIuMExSt454Oswr5Y3lCgzMRnh+rMqt/0zURtDdNa1wKZ+MKG4H72PWujI0bGYGyaV71jnjLLmeHHcrI4WkZsot+sKSsbSv7hHoZHGv1Us9ybjqfdIrdSoFvFKgjdiRCUpxuciN2TQ/vVpZXQx34Xvnw2msl02vM9ZtZLyjuVl+FQgqrjehBl12wTh7Ae349z9DqMdKVqAla6LGXHjW89JyLQnkkbD6+sj+hvhk1HwrSQV3DImJZYdtXdW27CFXtQwB1QDIIBFKHiF0vUCMHH4T2AuuuSRueNRhDkAp5YJfmPzLlow2ERMLP6v/nI2OVI7Jts2TwCXW0mOUc0eCkIOQ8qJ1ti6HH9eUqAo64VVAiUtAU7gOPK6PDYzRY7UlM6ALa+C1ewdpZ3+3ZXqTRoWxwDChvgSSe5jSn7fSv76CJQdbURP8mFBS0ATOWDOv+sG6lcb61Z37hgW4KfnKVeOCx0tJ0YB3FI/j9N0XvAmkWY+ICwRe7Xlsu/Pnh+huTnPpu/re6FFr+R+dqb6VuZ8EvEwbvF30abMcZscq7DKG9BrL83mo/4SgYZo13aNZzqK3YfmN8xhxWfb9qJXcRUtWUIDS4zu+t7AsKk/06qMmALRvkFqeV6VVCQYeHQ6LHli7ZrnEK3ADgpBG8ncVLx92QZwR1KaALTg6nPz/wWeRh3CkLibiRCPCYxxzwdhf5Xl3MVGbbPsDI2KKlLM3jS1fa64hZn3DD49PeqIQENwq1k=)";

    string cipherText_OFB_PKCS7_BASE64 =
        R"(EV+kgMV5PlIQhhbGHgeJq8qJK0ECb+e62UnJ/rxUOR6D6xUEt7iCoCRv3Povi6mTxM4qoEByhE/Lq2QHLEfbdWf/0IIUh9F/61VSHD4wr/djV0MODOOj/0acVkrUwzEtile/5r4MRiocyFdZ05locYlROr55mopisjy4kPxHVwBqJT+UK1Aw7cZ5jjLQmYrl2fmWGrbR3m6Ek54nR0qd5hXdB+JeWv+aQq22zRVT1Te565yDixgCXzcysC/JB1jKflTdchnvIQ1cK5ik9t7LMLvmq+szxKGfJhR1KdQkZugEPA7sMbQKzuuVG2IVuVz7CiEJG5f3Zx37rSB2uT6VsQ479Fr0StWoED9pUPFRRBUdj4I6n3Cwqth8FAwDHfPw+2DDzVwLSsq2quVvxmzfrfXgebf+bw6NFHro1N0lSjJZMT8YGRgLzKncs3ZTYwOPuoPzC0qRiQ8SStmEWoDGbt70xUYHFc5oB5UdxMGjoGA6MjRwgkZQPgpcxcS6XIYSVWaOb50zHJ0Pt5IEcNVXyLj5miclu14JUD4ZlPQ7Z1+z3Y3z+wmyXq+tHrNFxogZJh2+U62Mm7A3LiX0M5i8tjOPW7R381OwjGB8mheCnQMXoCF1FIB8y3Q7vVt/cLgCvr/O/L+Yft+xpGdFyby9uqHwXTqGql2+crkwsF8QZlF9Fr1mnWUeCEGXRvDhv/biVy+CPYUjHnI83DHxavW5yEM+5GeOdmHs51KBbOZiIHDiWm3whPrM+lM5IL73GQpOVCSwcozkQmPbCqn2V4yJyxOTIluydI4mwT45JV45xQUeMGTHpGXTbcQ3bbB0Au0WJvhG0n2AEaPyQYKNAcrBPSLBnR/BOG3T7wQcAPf9TPvsBDxbE557LevLfNrISDvUaApjCShera4gkVpL7XW32FG8nP7cQU8bbi298ybai4o6pPa1pzMp52BgAvv0YBOei1V6zVWGlLTDxOPhMTXKQbxBPwtxQFqAqLB4J9nAeLtAghDZjA70VDURXYn60IB/p6Q0qFyP5RjbtdZ8QzL+zPEEyk3T31mLAkvRMTyu05E3gLTziQJaYwalIUeGwyuAYot69sbR6zhfVik8h6Ud5O3ZlNZCEaD3QoZb7zzvACRDcWZQddwaimVqhasIrKL9eaTyTqdAcizea2FnQzTPyAqxRT0LuG0ZTMU65fIDYzUMZ/p+mjQi5jaRXV0pZbbMCM6b18GXIhglNofAXQl8RgeebrFMVmTQmpvjpcUV0TM=)";

    Base64Decrypt base64Decryption(false);

    base64Decryption.Update(cipherText_CFB_PKCS7_BASE64);
    string cipherText_CFB_PKCS7 = base64Decryption.GetString();
    base64Decryption.Reset();

    base64Decryption.Update(cipherText_CFB_ISO10126_BASE64);
    string cipherText_CFB_ISO10126 = base64Decryption.GetString();
    base64Decryption.Reset();

    base64Decryption.Update(cipherText_CFB_ANSIX923_BASE64);
    string cipherText_CFB_ANSIX923 = base64Decryption.GetString();
    base64Decryption.Reset();

    base64Decryption.Update(cipherText_ECB_PKCS7_BASE64);
    string cipherText_ECB_PKCS7 = base64Decryption.GetString();
    base64Decryption.Reset();

    base64Decryption.Update(cipherText_ECB_ANSIX923_BASE64);
    string cipherText_ECB_ANSIX923 = base64Decryption.GetString();
    base64Decryption.Reset();

    base64Decryption.Update(cipherText_CBC_PKCS7_BASE64);
    string cipherText_CBC_PKCS7 = base64Decryption.GetString();
    base64Decryption.Reset();

    base64Decryption.Update(cipherText_CTR_PKCS7_BASE64);
    string cipherText_CTR_PKCS7 = base64Decryption.GetString();
    base64Decryption.Reset();

    base64Decryption.Update(cipherText_OFB_PKCS7_BASE64);
    string cipherText_OFB_PKCS7 = base64Decryption.GetString();
    base64Decryption.Reset();

    {
        AesDecrypt decrypt(AESMode::CFB, AESPadding::PKCS7, key, iv);
        AesEncrypt encrypt(AESMode::CFB, AESPadding::PKCS7, key, iv);
        DoCryptoGTest(encrypt, decrypt, planText, cipherText_CFB_PKCS7);
    }
    {
        //ISO10126填充涉及到随机数，所以不能比较密文
        AesDecrypt decrypt(AESMode::CFB, AESPadding::ISO_10126, key, iv);
        AesEncrypt encrypt(AESMode::CFB, AESPadding::ISO_10126, key, iv);
        encrypt.Update(planText);
        auto iso10126CipherText = encrypt.GetString();
        decrypt.Update(iso10126CipherText);
        auto ISO0126PlanText = decrypt.GetString();
        EXPECT_EQ(ISO0126PlanText, planText);
    }
    {
        AesDecrypt decrypt(AESMode::CFB, AESPadding::ANSI_X_923, key, iv);
        AesEncrypt encrypt(AESMode::CFB, AESPadding::ANSI_X_923, key, iv);
        DoCryptoGTest(encrypt, decrypt, planText, cipherText_CFB_ANSIX923);
    }

    {
        AesDecrypt decrypt(AESMode::ECB, AESPadding::PKCS7, key);
        AesEncrypt encrypt(AESMode::ECB, AESPadding::PKCS7, key);
        DoCryptoGTest(encrypt, decrypt, planText, cipherText_ECB_PKCS7);
    }
    {
        AesDecrypt decrypt(AESMode::ECB, AESPadding::ANSI_X_923, key);
        AesEncrypt encrypt(AESMode::ECB, AESPadding::ANSI_X_923, key);
        DoCryptoGTest(encrypt, decrypt, planText, cipherText_ECB_ANSIX923);
    }
    {
        AesDecrypt decrypt(AESMode::CBC, AESPadding::PKCS7, key, iv);
        AesEncrypt encrypt(AESMode::CBC, AESPadding::PKCS7, key, iv);
        DoCryptoGTest(encrypt, decrypt, planText, cipherText_CBC_PKCS7);
    }
    {
        AesDecrypt decrypt(AESMode::CTR, AESPadding::PKCS7, key, iv);
        AesEncrypt encrypt(AESMode::CTR, AESPadding::PKCS7, key, iv);
        DoCryptoGTest(encrypt, decrypt, planText, cipherText_CTR_PKCS7);
    }
    {
        AesDecrypt decrypt(AESMode::OFB, AESPadding::PKCS7, key, iv);
        AesEncrypt encrypt(AESMode::OFB, AESPadding::PKCS7, key, iv);
        DoCryptoGTest(encrypt, decrypt, planText, cipherText_OFB_PKCS7);
    }
}

TEST(Digest, Name)
{
    const std::string kSalt = "ZeusZEUS";

    {
        auto digest = std::make_shared<HMACDigest>(kSalt.data(), kSalt.size(), HMACDigestType::MD5);
        EXPECT_EQ("HMAC(MD5)", digest->Name());
    }
    {
        auto digest = std::make_shared<HMACDigest>(kSalt.data(), kSalt.size(), HMACDigestType::SHA256);
        EXPECT_EQ("HMAC(SHA256)", digest->Name());
    }
    {
        auto digest = std::make_shared<HMACDigest>(kSalt.data(), kSalt.size(), HMACDigestType::SHA1);
        EXPECT_EQ("HMAC(SHA1)", digest->Name());
    }
    {
        auto digest = std::make_shared<HMACDigest>(kSalt.data(), kSalt.size(), HMACDigestType::SHA224);
        EXPECT_EQ("HMAC(SHA224)", digest->Name());
    }
    {
        auto digest = std::make_shared<HMACDigest>(kSalt.data(), kSalt.size(), HMACDigestType::SHA256);
        EXPECT_EQ("HMAC(SHA256)", digest->Name());
    }
    {
        auto digest = std::make_shared<HMACDigest>(kSalt.data(), kSalt.size(), HMACDigestType::SHA384);
        EXPECT_EQ("HMAC(SHA384)", digest->Name());
    }
    {
        auto digest = std::make_shared<HMACDigest>(kSalt.data(), kSalt.size(), HMACDigestType::SHA512);
        EXPECT_EQ("HMAC(SHA512)", digest->Name());
    }

    {
        auto digest = std::make_shared<SHA1Digest>();
        EXPECT_EQ("SHA1", digest->Name());
    }
    {
        auto digest = std::make_shared<SHA224Digest>();
        EXPECT_EQ("SHA224", digest->Name());
    }
    {
        auto digest = std::make_shared<SHA256Digest>();
        EXPECT_EQ("SHA256", digest->Name());
    }
    {
        auto digest = std::make_shared<SHA384Digest>();
        EXPECT_EQ("SHA384", digest->Name());
    }
    {
        auto digest = std::make_shared<SHA512Digest>();
        EXPECT_EQ("SHA512", digest->Name());
    }
    {
        auto digest = std::make_shared<Md5Digest>();
        EXPECT_EQ("MD5", digest->Name());
    }
    {
        auto digest = std::make_shared<RawMd5Digest>();
        EXPECT_EQ("MD5", digest->Name());
    }
    {
        auto digest = std::make_shared<Crc32Digest>();
        EXPECT_EQ("CRC32", digest->Name());
    }
    {
        auto digest = std::make_shared<Crc64Ecma182Digest>();
        EXPECT_EQ("CRC64ECMA182", digest->Name());
    }
}

TEST(Crypt, Name)
{
    string key = "9c38fd2138ebda58da2a43ea008d86a9";
    string iv  = "0c86747b7460c619";
    {
        auto crypt = std::make_shared<AesEncrypt>(AESMode::CFB, AESPadding::PKCS7, key, iv);
        EXPECT_EQ("AES(CFB|PKCS7)", crypt->Name());
    }

    {
        auto crypt = std::make_shared<AesDecrypt>(AESMode::CFB, AESPadding::ANSI_X_923, key, iv);
        EXPECT_EQ("AES(CFB|ANSI_X_923)", crypt->Name());
    }
    {
        auto crypt = std::make_shared<Base64Encrypt>();
        EXPECT_EQ("BASE64", crypt->Name());
    }
    {
        auto crypt = std::make_shared<Base64Decrypt>();
        EXPECT_EQ("BASE64", crypt->Name());
    }
}