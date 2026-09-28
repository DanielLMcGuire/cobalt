// ++C
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#include <atexit.h>
#include <cio.h>
#include <parr.h>
#include <alloc.h>

extern int program(parr_t csArgs);

extern void xxc_io_init(void);
extern void sio_init(unsigned int out, unsigned int err);

#ifdef _WIN32
#include <windows.h>
#endif

void pluspluscBoot(int argc, char **argv)
{
    xxc_io_init();
#if defined(__linux__)
    sio_init(0, 0);
#elif _WIN32
    sio_init(STD_OUTPUT_HANDLE, STD_ERROR_HANDLE);
#endif

    parr_t args = parr_new((const void**)argv, (size_t)argc);

    int ret = program(args);
    
    parr_free(&args);
    exit(ret);
    __NORETURN__
}