#pragma once
#ifdef _WIN32
// NOLINTBEGIN(cppcoreguidelines-macro-usage,cert-dcl37-c,cert-dcl51-cpp) 没办法，要定义变量，只能用宏
#if !defined(_AMD64_) && !defined(_X86_) && !defined(_ARM_)
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
#define _AMD64_
#elif defined(i386) || defined(__i386) || defined(__i386__) || defined(__i386__) || defined(_M_IX86)
#define _X86_
#elif defined(__arm__) || defined(_M_ARM) || defined(_M_ARMT)
#define _ARM_
#endif
#include <windef.h>
#undef _AMD64_
#undef _X86_
#undef _ARM_
#else
#include <windef.h>
#endif
// NOLINTEND(cppcoreguidelines-macro-usage,cert-dcl37-c,cert-dcl51-cpp)
#endif