// ++C CRT
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#include <atexit.h>
#ifdef _WIN32
#include <windows.h>

static void rexit(int status)
{
    ExitProcess((UINT)status);
}
#elif defined(__linux__)
#include "sys_linux.h"

static void rexit(int status)
{
    sys_exit(status);
}
#endif

static atexit_func_t exit_handlers[MAX_ATEXIT_FUNCS];
static int handler_count = 0;

int atexit(atexit_func_t func)
{
    if (func == NULL || handler_count >= MAX_ATEXIT_FUNCS)
    {
        return -1; 
    }
    
    exit_handlers[handler_count++] = func;
    
    return 0; 
}

void exit(int status)
{
    for (int i = handler_count - 1; i >= 0; i--)
        if (exit_handlers[i] != NULL)
            exit_handlers[i]();

    rexit(status);
}