// ++C CRT
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#include <atexit.h>
#include <lock.h>
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

static crt_lock_t g_atexit_lock = CRT_LOCK_INIT;
static bool g_exiting = false;

int atexit(atexit_func_t func)
{
    if (func == NULL)
        return -1;

    crt_lock_acquire(&g_atexit_lock);

    if (handler_count >= MAX_ATEXIT_FUNCS)
    {
        crt_lock_release(&g_atexit_lock);
        return -1;
    }

    exit_handlers[handler_count++] = func;

    crt_lock_release(&g_atexit_lock);

    return 0;
}

void exit(int status)
{
    crt_lock_acquire(&g_atexit_lock);

    if (g_exiting)
    {
        crt_lock_release(&g_atexit_lock);
        __NORETURN__
    }

    g_exiting = true;

    crt_lock_release(&g_atexit_lock);

    for (;;)
    {
        atexit_func_t func;

        crt_lock_acquire(&g_atexit_lock);

        if (handler_count <= 0)
        {
            crt_lock_release(&g_atexit_lock);
            break;
        }

        func = exit_handlers[--handler_count];

        crt_lock_release(&g_atexit_lock);

        if (func != NULL)
            func();
    }

    rexit(status);
}