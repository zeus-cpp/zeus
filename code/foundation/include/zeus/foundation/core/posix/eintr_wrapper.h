#pragma once

#ifdef __linux__
#include <errno.h>

#ifndef HANDLE_EINTR

#define HANDLE_EINTR(x)                                     \
    __extension__({                                         \
        decltype(x) eintr_wrapper_result;                   \
        do                                                  \
        {                                                   \
            eintr_wrapper_result = (x);                     \
        }                                                   \
        while (eintr_wrapper_result < 0 && errno == EINTR); \
        eintr_wrapper_result;                               \
    })

#endif

#ifndef IGNORE_EINTR

#define IGNORE_EINTR(x)                                     \
    __extension__({                                         \
        decltype(x) eintr_wrapper_result;                   \
        do                                                  \
        {                                                   \
            eintr_wrapper_result = (x);                     \
            if (eintr_wrapper_result < 0 && errno == EINTR) \
            {                                               \
                eintr_wrapper_result = 0;                   \
            }                                               \
        }                                                   \
        while (0);                                          \
        eintr_wrapper_result;                               \
    })

#endif

#endif