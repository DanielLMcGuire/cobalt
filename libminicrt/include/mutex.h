// ++C C Runtime Library (libminicrt)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#ifndef MUTEX_H
#define MUTEX_H

#include <def.h>
#include <crt_lock.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    MUTEX_PLAIN      = 0,
    MUTEX_RECURSIVE  = 1,
    MUTEX_ERRORCHECK = 2
};

enum {
    MUTEX_SUCCESS =  0,
    MUTEX_ERROR   = -1,
    MUTEX_BUSY    = -2,
    MUTEX_EDEADLK = -3,
    MUTEX_EPERM   = -4
};

typedef struct {
    crt_lock_t   lock;
    volatile long owner_tid;
    u32           count;
    u8            type;
} mutex_t;

#define MUTEX_INIT             { .lock = CRT_LOCK_INIT, .owner_tid = 0, .count = 0, .type = MUTEX_PLAIN }
#define MUTEX_RECURSIVE_INIT   { .lock = CRT_LOCK_INIT, .owner_tid = 0, .count = 0, .type = MUTEX_RECURSIVE }
#define MUTEX_ERRORCHECK_INIT  { .lock = CRT_LOCK_INIT, .owner_tid = 0, .count = 0, .type = MUTEX_ERRORCHECK }

int      mutex_init(mutex_t *mtx, int type);
int      mutex_destroy(mutex_t *mtx);

mutex_t *mutex_create(int type);
void     mutex_free(mutex_t *mtx);

int      mutex_lock(mutex_t *mtx);
int      mutex_trylock(mutex_t *mtx);
int      mutex_unlock(mutex_t *mtx);

bool     mutex_is_locked(const mutex_t *mtx);
bool     mutex_is_owned_by_current_thread(const mutex_t *mtx);
long     mutex_get_owner(const mutex_t *mtx);
u32      mutex_get_count(const mutex_t *mtx);

#define MUTEX_GUARD(mtx) \
    for (int _guard_##__LINE__ = (mutex_lock(mtx), 1); \
         _guard_##__LINE__; \
         _guard_##__LINE__ = 0, mutex_unlock(mtx))

#if defined(__GNUC__) || defined(__clang__)
static inline void __mutex_scoped_unlock(mutex_t **pmtx)
{
    if (pmtx && *pmtx)
        mutex_unlock(*pmtx);
}

#define MUTEX_SCOPED(mtx) \
    mutex_lock(mtx); \
    __attribute__((cleanup(__mutex_scoped_unlock))) mutex_t *__scoped_##__LINE__ = (mtx)
#endif

#ifdef __cplusplus
}
#endif

#endif /* MUTEX_H */