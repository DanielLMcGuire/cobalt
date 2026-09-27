// ++C CRT | Platform (Linux)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#include "../linux_alloc.h"

__linux_arena *__linux_arenas = NULL;
__linux_block *__linux_free_list = NULL;

int __linux_alloc_lock = 0;

size_t __linux_align_up(size_t size)
{
    size_t mask = (size_t)__LINUX_ALIGNMENT - 1;

    if (size > (size_t)-1 - mask)
        return 0;

    return (size + mask) & ~mask;
}

size_t __linux_block_header_size(void)
{
    return __linux_align_up(sizeof(__linux_block));
}

size_t __linux_arena_header_size(void)
{
    return __linux_align_up(sizeof(__linux_arena));
}

size_t __linux_block_size(const __linux_block *block)
{
    return block->size_flags & ~(size_t)1;
}

int __linux_block_is_free(const __linux_block *block)
{
    return (block->size_flags & 1) == 0;
}

void __linux_add_free(__linux_block *block)
{
    block->next_free = __linux_free_list;
    __linux_free_list = block;
}

void __linux_remove_free(__linux_block *target)
{
    __linux_block *previous = NULL;
    __linux_block *current = __linux_free_list;

    while (current)
    {
        if (current == target)
        {
            if (previous)
                previous->next_free = current->next_free;
            else
                __linux_free_list = current->next_free;

            current->next_free = NULL;
            return;
        }

        previous = current;
        current = current->next_free;
    }
}

__linux_arena *__linux_find_arena(const __linux_block *block)
{
    size_t arena_header = __linux_arena_header_size();
    __linux_arena *arena = __linux_arenas;

    while (arena)
    {
        const char *start = (const char *)arena + arena_header;
        const char *end = (const char *)arena + arena->size;

        if ((const char *)block >= start &&
            (const char *)block < end)
            return arena;

        arena = arena->next;
    }

    return NULL;
}

void __linux_coalesce_and_free(__linux_block *block, __linux_arena *arena)
{
    size_t block_size = __linux_block_size(block);
    size_t arena_header = __linux_arena_header_size();
    char *arena_start = (char *)arena;
    char *arena_end = arena_start + arena->size;

    block->size_flags = block_size;
    block->next_free = NULL;

    __linux_block *next = (__linux_block *)((char *)block + block_size);

    if ((char *)next < arena_end && __linux_block_is_free(next))
    {
        __linux_remove_free(next);

        block_size += __linux_block_size(next);
        block->size_flags = block_size;
    }

    if (block->prev_size != 0)
    {
        __linux_block *previous = (__linux_block *)((char *)block - block->prev_size);

        if (__linux_block_is_free(previous))
        {
            __linux_remove_free(previous);

            block_size = __linux_block_size(previous) + block_size;

            previous->size_flags = block_size;
            block = previous;
        }
    }

    next = (__linux_block *)((char *)block + block_size);

    if ((char *)next < arena_end)
        next->prev_size = block_size;

    if ((char *)block == arena_start + arena_header &&
        block_size == arena->size - arena_header)
        {

        __linux_remove_free(block);

        __linux_arena *previous_arena = NULL;
        __linux_arena *current_arena = __linux_arenas;

        while (current_arena) 
        {
            if (current_arena == arena)
            {
                if (previous_arena)
                    previous_arena->next = current_arena->next;
                else
                    __linux_arenas = current_arena->next;

                sys_munmap(arena, arena->size);
                return;
            }

            previous_arena = current_arena;
            current_arena = current_arena->next;
        }

        __linux_add_free(block);
        return;
    }

    __linux_add_free(block);
}