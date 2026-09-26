// ++C CRT
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#ifndef ALLOC_FUNCS_H
#define ALLOC_FUNCS_H

#include <def.h>

void* malloc(size_t size);

void* calloc(size_t count, size_t size);

void* realloc(void *ptr, size_t size);

void free(void *ptr);

#endif