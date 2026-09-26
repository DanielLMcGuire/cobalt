// ++C
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#include <str.h>

size_t strlen(const char *s)
{
    size_t len = 0;
    while (s && s[len]) len++;
    return len;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}