// ++C C Runtime Library (libminicrt)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#ifndef THREAD_H
#define THREAD_H

#include <def.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xxc_thread *thread_t;
typedef void *(*thread_fn_t)(void *arg);

typedef struct {
    size_t stack_size;
    bool   detached;
} thread_attr_t;

#define THREAD_ATTR_INIT { 0, false }
#define THREAD_DEFAULT_STACK_SIZE ((size_t)1024 * 1024)

enum {
    THREAD_SUCCESS =  0,
    THREAD_ERROR   = -1,
    THREAD_NOMEM   = -2,
    THREAD_INVAL   = -3,
    THREAD_DEADLK  = -4
};

int  thread_create(thread_t *out, thread_fn_t fn, void *arg, const thread_attr_t *attr);
int  thread_join(thread_t t, void **retval);
int  thread_detach(thread_t t);

thread_t thread_self(void);
bool     thread_equal(thread_t a, thread_t b);
u32      thread_current_id(void);
u32      thread_id(thread_t t);

void     XXC_NORETURN thread_exit(void *retval);

void     thread_yield(void);
void     thread_sleep_ms(u32 ms);

typedef u32 tls_key_t;
#define TLS_KEYS_MAX 64

int   tls_key_create(tls_key_t *key, void (*destructor)(void *));
int   tls_key_delete(tls_key_t key);
void *tls_get(tls_key_t key);
int   tls_set(tls_key_t key, void *value);

#ifdef __cplusplus
}
#endif

#endif /* THREAD_H */
