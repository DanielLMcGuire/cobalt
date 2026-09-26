// ++C CRT | Platform (Linux)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#ifndef LINUX_ALLOC_H
#define LINUX_ALLOC_H
#include "../libminicrt/include/def.h"
#include "sys_linux.h"
enum {
    __LINUX_ARENA_SIZE = 64 * 1024,
    __LINUX_ALIGNMENT = 16
};
typedef struct __linux_block __linux_block;
typedef struct __linux_arena __linux_arena;
struct __linux_block {
    size_t size_flags;
    size_t prev_size;
    __linux_block *next_free;
};
struct __linux_arena {
    size_t size;
    __linux_arena *next;
};
extern __linux_arena *__linux_arenas;
extern __linux_block *__linux_free_list;
extern int __linux_alloc_lock;

static inline void __linux_lock(void)
{
    int expected = 0;
    if (__atomic_compare_exchange_n(&__linux_alloc_lock, &expected, 1, 0, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
        return;

    for (int i = 0; i < 40; i++)
    {
        expected = 0;
        if (__linux_alloc_lock == 0 && __atomic_compare_exchange_n(&__linux_alloc_lock, &expected, 1, 0, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
            return;

#if defined(__x86_64__) || defined(__i386__)
        __asm__ volatile("pause");
#elif defined(__aarch64__) || defined(__arm__)
        __asm__ volatile("yield");
#endif
    }

    int c = __atomic_exchange_n(&__linux_alloc_lock, 2, __ATOMIC_ACQUIRE);
    while (c != 0)
    {
        sys_futex(&__linux_alloc_lock, FUTEX_WAIT_PRIVATE, 2, NULL, NULL, 0);
        c = __atomic_exchange_n(&__linux_alloc_lock, 2, __ATOMIC_ACQUIRE);
    }
}

static inline void __linux_unlock(void)
{
    if (__atomic_fetch_sub(&__linux_alloc_lock, 1, __ATOMIC_RELEASE) != 1)
    {
        __atomic_store_n(&__linux_alloc_lock, 0, __ATOMIC_RELEASE);
        sys_futex(&__linux_alloc_lock, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
    }
}

size_t __linux_align_up(size_t size);
size_t __linux_block_header_size(void);
size_t __linux_arena_header_size(void);
size_t __linux_block_size(const __linux_block *block);
int __linux_block_is_free(const __linux_block *block);
void __linux_add_free(__linux_block *block);
void __linux_remove_free(__linux_block *target);
__linux_arena *__linux_find_arena(const __linux_block *block);
void __linux_coalesce_and_free(__linux_block *block, __linux_arena *arena);
#endif // LINUX_ALLOC_H