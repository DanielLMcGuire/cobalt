// ++C C Runtime Library (libminicrt)
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

void* memmove(void* dest, const void* src, size_t n)
{
    char* d = (char*)dest;
    const char* s = (const char*)src;
    if (d == s || n == 0)
        return dest;
    if (d < s)
    {
        for (size_t i = 0; i < n; i++)
            d[i] = s[i];
    }
    else
    {
        for (size_t i = n; i > 0; i--)
            d[i - 1] = s[i - 1];
    }
    return dest;
}

int memcmp(const void* a, const void* b, size_t n)
{
    const unsigned char* pa = (const unsigned char*)a;
    const unsigned char* pb = (const unsigned char*)b;
    for (size_t i = 0; i < n; i++)
    {
        if (pa[i] != pb[i])
            return (int)pa[i] - (int)pb[i];
    }
    return 0;
}

#if defined(_MSC_VER) && !defined(__clang__)
#pragma optimize("", on)
#endif