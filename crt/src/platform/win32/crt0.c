extern void* malloc(size_t size);
extern void free(void *ptr);


#define va_start vastart_guard
#include <windows.h>
#undef va_start
#include "../../../include/def.h"
#include "commandline.h"

extern void exit(int status);
typedef void (*atexit_func_t)(void);
extern int atexit(atexit_func_t func);

extern void pluspluscBoot(int argc, char **argv);

extern HANDLE heap;

int argc = 0;
char **argv = NULL;

void freeArgs(void)
{
    FreeArgvA(argc, argv);
}

void start(void)
{
    heap = GetProcessHeap();
    argv = GetArgvA(&argc);
    atexit(freeArgs);
    pluspluscBoot(argc, argv);
    __NORETURN__
}