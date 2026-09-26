// ++C CRT
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#include <alloc.h>
#include <def.h>
#include <mem.h>
#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <sys_linux.h>
#include <linux_alloc.h>
#endif

#ifdef _WIN32

HANDLE heap = INVALID_HANDLE_VALUE;

void* malloc(size_t size)
{
    if (size == 0) size = 1;
    return HeapAlloc(heap, 0, (SIZE_T)size);
}
#elif defined(__linux__)
void* malloc(size_t size)
{
    __linux_lock();
    size_t header = __linux_block_header_size();

    if (size == 0)
        size = 1;

    if (size > (size_t)-1 - header - ((size_t)__LINUX_ALIGNMENT - 1))
    {
        __linux_unlock();
        return NULL;
    }

    size_t needed = __linux_align_up(size + header);

    if (needed == 0)
    {
        __linux_unlock();
        return NULL;
    }

    if (needed > __LINUX_ARENA_SIZE / 2)
    {
        void *memory = sys_mmap(NULL, needed, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

        if (!memory)
        {
            __linux_unlock();
            return NULL;
        }

        __linux_block *block = (__linux_block *)memory;

        block->size_flags = needed | 1;
        block->prev_size = 1;
        block->next_free = NULL;

        __linux_unlock();
        return (char *)block + header;
    }

    __linux_block *block = __linux_free_list;
    __linux_block *previous_free = NULL;

    while (block)
    {
        if (__linux_block_size(block) >= needed)
            break;

        previous_free = block;
        block = block->next_free;
    }

    if (!block)
    {
        size_t arena_header = __linux_arena_header_size();
        size_t arena_size = __LINUX_ARENA_SIZE;

        if (arena_size < arena_header + needed)
        {
            if (needed > (size_t)-1 - arena_header)
            {
                __linux_unlock();
                return NULL;
            }

            arena_size = __linux_align_up(
                arena_header + needed
            );

            if (arena_size == 0)
            {
                __linux_unlock();
                return NULL;
            }
        }

        void *memory = sys_mmap(NULL, arena_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

        if (!memory)
        {
            __linux_unlock();
            return NULL;
        }

        __linux_arena *arena = (__linux_arena *)memory;

        arena->size = arena_size;
        arena->next = __linux_arenas;
        __linux_arenas = arena;

        block = (__linux_block *)((char *)arena + arena_header);

        block->size_flags = (arena_size - arena_header) & ~((size_t)__LINUX_ALIGNMENT - 1);

        block->prev_size = 0;
        block->next_free = NULL;

        __linux_free_list = block;
        previous_free = NULL;
    }

    size_t block_size = __linux_block_size(block);
    size_t remaining = block_size - needed;

    if (remaining >= header + __LINUX_ALIGNMENT)
    {
        __linux_block *split = (__linux_block *)((char *)block + needed);

        split->size_flags = remaining;
        split->prev_size = needed;
        split->next_free = block->next_free;

        if (previous_free)
            previous_free->next_free = split;
        else
            __linux_free_list = split;

        __linux_arena *arena = __linux_find_arena(block);

        if (arena)
        {
            char *end = (char *)arena + arena->size;

            __linux_block *after = (__linux_block *)((char *)split + remaining);

            if ((char *)after < end)
                after->prev_size = remaining;
        }
    } else {
        if (previous_free)
            previous_free->next_free = block->next_free;
        else
            __linux_free_list = block->next_free;
    }

    block->size_flags = needed | 1;
    block->next_free = NULL;

    __linux_unlock();
    return (char *)block + header;
}
#endif

void* calloc(size_t count, size_t size)
{
    size_t total = count * size;

    if (count != 0 && total / count != size)
        return NULL;
#ifdef _WIN32
    return HeapAlloc(heap, HEAP_ZERO_MEMORY, (SIZE_T)total);
#elif defined(__linux__)
    void *ptr = malloc(total);
    if (ptr) memset(ptr, 0, total);
    return ptr;
#endif
}

#ifdef _WIN32
void* realloc(void *ptr, size_t size)
{
    if (!ptr)
    {
        if (size == 0) size = 1;
        return HeapAlloc(heap, 0, (SIZE_T)size);
    }

    if (size == 0)
    {
        HeapFree(heap, 0, ptr);
        return NULL;
    }

    return HeapReAlloc(heap, 0, ptr, (SIZE_T)size);
}
#elif defined(__linux__)
void* realloc(void *ptr, size_t size)
{
    __linux_lock();
    size_t header = __linux_block_header_size();

    if (!ptr) 
    {
        __linux_unlock();
        return malloc(size);
    }

    if (size == 0) 
    {
        __linux_unlock();
        free(ptr);
        return NULL;
    }

    if (size > (size_t)-1 - header - ((size_t)__LINUX_ALIGNMENT - 1)) 
    {
        __linux_unlock();
        return NULL;
    }

    size_t needed = __linux_align_up(size + header);

    if (needed == 0)
    {
        __linux_unlock();
        return NULL;
    }

    __linux_block *block = (__linux_block *)((char *)ptr - header);

    size_t old_block_size = __linux_block_size(block);

    size_t old_usable = old_block_size - header;

    if (needed <= old_block_size)
    {
        if (block->prev_size == 1)
        {
            __linux_unlock();
            return ptr;
        }

        size_t remaining =
            old_block_size - needed;

        if (remaining >= header + __LINUX_ALIGNMENT)
        {
            __linux_arena *arena = __linux_find_arena(block);

            if (arena)
            {
                char *end = (char *)arena + arena->size;

                block->size_flags = needed | 1;

                __linux_block *tail = (__linux_block *)((char *)block + needed);

                tail->size_flags = remaining;
                tail->prev_size = needed;
                tail->next_free = NULL;

                __linux_block *after = (__linux_block *)((char *)tail + remaining);

                if ((char *)after < end)
                    after->prev_size = remaining;

                __linux_coalesce_and_free(tail, arena);
            }
        }

        __linux_unlock();
        return ptr;
    }

    if (block->prev_size != 1)
    {
        __linux_arena *arena =
            __linux_find_arena(block);

        if (arena)
        {
            char *end = (char *)arena + arena->size;

            __linux_block *next = (__linux_block *)((char *)block + old_block_size);

            if ((char *)next < end && __linux_block_is_free(next))
                {

                size_t combined = old_block_size + __linux_block_size(next);

                if (combined >= needed)
                {
                    __linux_remove_free(next);

                    size_t remaining = combined - needed;

                    if (remaining >=header + __LINUX_ALIGNMENT)
                    {

                        block->size_flags = needed | 1;

                        __linux_block *tail = (__linux_block *)((char *)block + needed);
                        tail->size_flags = remaining;
                        tail->prev_size = needed;
                        tail->next_free = NULL;

                        __linux_block *after = (__linux_block *)((char *)tail + remaining);

                        if ((char *)after < end) 
                            after->prev_size = remaining;

                        __linux_coalesce_and_free(tail, arena);
                    }
                    else
                    {
                        block->size_flags =
                            combined | 1;

                        __linux_block *after = (__linux_block *)((char *)block + combined);

                        if ((char *)after < end)
                            after->prev_size = combined;
                    }
                    __linux_unlock();
                    return ptr;
                }
            }
        }
    }

    __linux_unlock();

    void *new_ptr = malloc(size);

    if (!new_ptr)
        return NULL;

    memcpy(new_ptr, ptr, old_usable < size ? old_usable : size);

    free(ptr);

    return new_ptr;
}
#endif

#ifdef _WIN32
void free(void *ptr) 
{
    if (!ptr) 
        return;

    HeapFree(heap, 0, ptr);
}
#elif defined(__linux__)
void free(void *ptr)
{
    __linux_lock();
    size_t header = __linux_block_header_size();

    __linux_block *block =
        (__linux_block *)((char *)ptr - header);

    if (block->prev_size == 1)
    {
        sys_munmap(block, __linux_block_size(block));
        __linux_unlock();
        return;
    }

    __linux_arena *arena =
        __linux_find_arena(block);

    if (!arena)
    {
        __linux_unlock();
        return;
    }

    __linux_coalesce_and_free(block, arena);
    __linux_unlock();
}
#endif
