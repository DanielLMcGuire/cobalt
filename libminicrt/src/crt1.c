// ++C
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#include <atexit.h>
#include <sio.h>
#include <parr.h>
#include <alloc.h>

extern int program(parr_t csArgs);

extern void fs_init(void);
extern void init_io(unsigned int out, unsigned int err);

#ifdef _WIN32
#include <windows.h>

void pluspluscBoot(int argc, char **argv)
{
    fs_init();
    init_io(STD_OUTPUT_HANDLE, STD_ERROR_HANDLE);

    parr_t args = parr_new((const void**)argv, (size_t)argc);

    int ret = program(args);
    
    parr_free(&args);
    exit(ret);
    __NORETURN__
}

#elif defined(__linux__)
#include "sys_linux.h"

void pluspluscBoot(int argc, char **argv)
{
    fs_init();
    init_io(0, 0);
    parr_t args = parr_new((const void**)argv, (size_t)argc);
    int ret = program(args);
    parr_free(&args);
    exit(ret);
    __NORETURN__
}

#endif