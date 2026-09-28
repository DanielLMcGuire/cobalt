// ++C CRT
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#ifndef ALLOC_FUNCS_H
#define ALLOC_FUNCS_H

#include <def.h>

// allocate in memory
void* malloc(size_t size);
void* calloc(size_t count, size_t size);
void* realloc(void *ptr, size_t size);

// free memory
void free(void *ptr);

// allocate on the stack
#if defined(__clang__) || defined(__GNUC__)
    #define alloca(size) __builtin_alloca(size)
#elif defined(_MSC_VER)
    void* __cdecl _alloca(size_t);
    #pragma intrinsic(_alloca)
    #define alloca(size) _alloca(size)
#else
    #error "Unsupported compiler for alloca"
#endif

#endif