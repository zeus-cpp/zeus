#pragma once

#include <cstdint>
#include <string>

namespace zeus
{

uint8_t RandUint8();
uint8_t RandUint8(uint8_t max);
uint8_t RandUint8(uint8_t min, uint8_t max);

int8_t RandInt8();
int8_t RandInt8(int8_t max);
int8_t RandInt8(int8_t min, int8_t max);

uint16_t RandUint16();
uint16_t RandUint16(uint16_t max);
uint16_t RandUint16(uint16_t min, uint16_t max);

int16_t RandInt16();
int16_t RandInt16(int16_t max);
int16_t RandInt16(int16_t min, int16_t max);

uint32_t RandUint32();
uint32_t RandUint32(uint32_t max);
uint32_t RandUint32(uint32_t min, uint32_t max);

int32_t RandInt32();
int32_t RandInt32(int32_t max);
int32_t RandInt32(int32_t min, int32_t max);

uint64_t RandUint64();
uint64_t RandUint64(uint64_t max);
uint64_t RandUint64(uint64_t min, uint64_t max);

int64_t RandInt64();
int64_t RandInt64(int64_t max);
int64_t RandInt64(int64_t min, int64_t max);

float RandFloat();
float RandFloat(float max);
float RandFloat(float min, float max);

double RandDouble();
double RandDouble(double max);
double RandDouble(double min, double max);

bool RandBool();

char        RandLetter();
char        RandLowerLetter();
char        RandUpperLetter();
char        RandNumberLetter();
char        RandHex();
void        RandBytes(void* output, size_t size);
std::string RandWord();
std::string RandString(size_t size);
std::string RandIpV4String();
} // namespace zeus

#include "zeus/foundation/core/zeus_compatible.h"
