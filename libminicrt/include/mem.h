// ++C CRT
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#ifndef MEMORY_H
#define MEMORY_H

#include <def.h>

void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* dest, int c, size_t n);

#endif /* MEMORY_H */