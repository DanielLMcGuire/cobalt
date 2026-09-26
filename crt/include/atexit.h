// ++C CRT
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#ifndef ATEXIT_H
#define ATEXIT_H
#include <def.h>

#ifndef MAX_ATEXIT_FUNCS
#define MAX_ATEXIT_FUNCS 128
#endif

typedef void (*atexit_func_t)(void);

int atexit(atexit_func_t func);

void exit(int status);

#endif /* ATEXIT_H */