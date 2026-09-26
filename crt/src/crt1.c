// ++C
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#include <atexit.h>
#include <sio.h>
#include <fio.pph>
#include <parr.h>
#include <alloc.h>

extern int program(parr_t csArgs);

#ifdef _WIN32
#include <windows.h>

void setupIO()
{
    FileStream *stdout_fs = NEW(FileStream);
    FileStream *stderr_fs = NEW(FileStream);
    FileStream *stdin_fs  = NEW(FileStream);

    stdout_fs->fd = GetStdHandle(STD_OUTPUT_HANDLE);
    stderr_fs->fd = GetStdHandle(STD_ERROR_HANDLE);
    stdin_fs->fd  = GetStdHandle(STD_INPUT_HANDLE);

    stdout_fs->owns_fd = false;
    stderr_fs->owns_fd = false;
    stdin_fs->owns_fd  = false;

    fs_init(stdin_fs, stdout_fs, stderr_fs);
}

void pluspluscBoot(int argc, char **argv)
{
    init_io(STD_OUTPUT_HANDLE, STD_ERROR_HANDLE);

    setupIO();

    parr_t args = parr_new((const void**)argv, (size_t)argc);

    int ret = program(args);
    
    parr_free(&args);
    exit(ret);
    __NORETURN__
}

#elif defined(__linux__)
#include "sys_linux.h"

void setupIO()
{
    FileStream *stdout_fs = NEW(FileStream);
    FileStream *stderr_fs = NEW(FileStream);
    FileStream *stdin_fs  = NEW(FileStream);

    stdout_fs->fd = 1;
    stderr_fs->fd = 2;
    stdin_fs->fd  = 0;

    stdout_fs->owns_fd = false;
    stderr_fs->owns_fd = false;
    stdin_fs->owns_fd  = false;

    fs_init(stdin_fs, stdout_fs, stderr_fs);
}

void pluspluscBoot(int argc, char **argv)
{
    init_io(0, 0);
    parr_t args = parr_new((const void**)argv, (size_t)argc);
    int ret = program(args);
    parr_free(&args);
    exit(ret);
    __NORETURN__
}

#endif