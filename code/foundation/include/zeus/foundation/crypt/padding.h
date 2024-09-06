#pragma once
#include <cstdint>
#include <cstddef>

namespace zeus
{

//所有Fill函数的buffer的长度不能小于dataLength+blockLength

/// @brief 填充PKCS7格式Padding
/// @param buffer 数据缓冲区
/// @param dataLength 数据缓冲区的长度
/// @param dataLength 需要进行Padding的数据的长度
/// @param blockLength 想要填充对齐的对齐长度
/// @return 填充的长度
std::size_t FillPKCS7Padding(uint8_t* buffer, std::size_t bufferLength, std::size_t dataLength, uint8_t blockLength);

/// @brief 查找PKCS7格式Padding
/// @param buffer 数据缓冲区
/// @param bufferLength 数据缓冲区的长度
/// @return 检测到的需要移除的Padding长度
std::size_t FindPKCS7Padding(const uint8_t* buffer, std::size_t bufferLength);
//使用方法同PKCS7
std::size_t FillANSI_X_923Padding(uint8_t* buffer, std::size_t bufferLength, std::size_t dataLength, uint8_t blockLength);
//使用方法同PKCS7
std::size_t FindANSI_X_923Padding(const uint8_t* buffer, std::size_t bufferLength);
//使用方法同PKCS7
std::size_t FillISO_10126Padding(uint8_t* buffer, std::size_t bufferLength, std::size_t dataLength, uint8_t blockLength);
//使用方法同PKCS7
std::size_t FindISO_10126Padding(const uint8_t* buffer, std::size_t bufferLength);
}
#include "zeus/foundation/core/zeus_compatible.h"
