// ++C CRT
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#include <mem.h>
#include <def.h>

#if defined(_MSC_VER) && !defined(__clang__)
#pragma function(memcpy)
#pragma function(memset)
#pragma optimize("", off) 
#endif

void* memcpy(void* dest, const void* src, size_t n)
{
    char* d = (char*)dest;
    const char* s = (const char*)src;
    for (size_t i = 0; i < n; i++)
        d[i] = s[i];
    return dest;
}

void* memset(void* dest, int c, size_t n)
{
    char* d = (char*)dest;
    for (size_t i = 0; i < n; i++)
        d[i] = (char)c;
    return dest;
}

#if defined(_MSC_VER) && !defined(__clang__)
#pragma optimize("", on)
#endif