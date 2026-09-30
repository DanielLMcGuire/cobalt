// ++C C Runtime Library (libminicrt)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#ifndef ATOMIC_H
#define ATOMIC_H

#include <def.h>

typedef enum {
    ATOMIC_RELAXED = 0,
    ATOMIC_CONSUME = 1,
    ATOMIC_ACQUIRE = 2,
    ATOMIC_RELEASE = 3,
    ATOMIC_ACQ_REL = 4,
    ATOMIC_SEQ_CST = 5
} atomic_order_t;

#if defined(__cplusplus) // for fixing LSP errors, not for C++ support
    #define XXC_ALIGNAS(n) alignas(n)
    extern "C" {
#else
    #define XXC_ALIGNAS(n) _Alignas(n)
#endif

#if (defined(__GNUC__) || defined(__clang__)) && !defined(XXC_ATOMIC_FORCE_MSVC)
    #define XXC_ATOMIC_GNU 1
#elif defined(_MSC_VER) || defined(XXC_ATOMIC_FORCE_MSVC)
    #define XXC_ATOMIC_MSVC 1
#else
    #error "atomic.h: unsupported compiler"
#endif

#if defined(XXC_ATOMIC_GNU)

    #define XXC_A_LOAD(B, P, ORD)               __atomic_load_n((P), (int)(ORD))
    #define XXC_A_STORE(B, P, V, ORD)           __atomic_store_n((P), (V), (int)(ORD))
    #define XXC_A_XCHG(B, P, V, ORD)            __atomic_exchange_n((P), (V), (int)(ORD))
    #define XXC_A_CAS(B, P, EP, D, WEAK, S, F)  __atomic_compare_exchange_n((P), (EP), (D), (WEAK), (int)(S), (int)(F))
    #define XXC_A_ADD(B, P, V, ORD)             __atomic_fetch_add((P), (V), (int)(ORD))
    #define XXC_A_SUB(B, P, V, ORD)             __atomic_fetch_sub((P), (V), (int)(ORD))
    #define XXC_A_AND(B, P, V, ORD)             __atomic_fetch_and((P), (V), (int)(ORD))
    #define XXC_A_OR(B, P, V, ORD)              __atomic_fetch_or((P), (V), (int)(ORD))
    #define XXC_A_XOR(B, P, V, ORD)             __atomic_fetch_xor((P), (V), (int)(ORD))

    static inline void atomic_fence(atomic_order_t order)
    {
        __atomic_thread_fence((int)order);
    }

    static inline void cpu_relax(void)
    {
    #if defined(__x86_64__) || defined(__i386__)
        __asm__ volatile("pause" ::: "memory");
    #elif defined(__aarch64__) || defined(__arm__)
        __asm__ volatile("yield" ::: "memory");
    #else
        __asm__ volatile("" ::: "memory");
    #endif
    }

#else /* XXC_ATOMIC_MSVC */

    #include <intrin.h>

    #define XXC_MSVC_ATOMIC_WIDTH(B, ST, CASFN)                                            \
        static inline ST xxc__a_cas_##B(volatile ST *p, ST expected, ST desired)           \
        {                                                                                  \
            return CASFN(p, desired, expected);                                            \
        }                                                                                  \
        static inline ST xxc__a_load_##B(volatile ST *p)                                   \
        {                                                                                  \
            XXC_MSVC_LOAD_BODY(B, ST)                                                      \
        }                                                                                  \
        static inline ST xxc__a_xchg_##B(volatile ST *p, ST v)                             \
        {                                                                                  \
            ST cur = xxc__a_load_##B(p);                                                   \
            for (;;)                                                                       \
            {                                                                              \
                ST seen = xxc__a_cas_##B(p, cur, v);                                       \
                if (seen == cur) return cur;                                               \
                cur = seen;                                                                \
            }                                                                              \
        }                                                                                  \
        static inline void xxc__a_store_##B(volatile ST *p, ST v)                          \
        {                                                                                  \
            (void)xxc__a_xchg_##B(p, v);                                                   \
        }                                                                                  \
        static inline ST xxc__a_add_##B(volatile ST *p, ST v)                              \
        {                                                                                  \
            ST cur = xxc__a_load_##B(p);                                                   \
            for (;;)                                                                       \
            {                                                                              \
                ST seen = xxc__a_cas_##B(p, cur, (ST)(cur + v));                           \
                if (seen == cur) return cur;                                               \
                cur = seen;                                                                \
            }                                                                              \
        }                                                                                  \
        static inline ST xxc__a_sub_##B(volatile ST *p, ST v)                              \
        {                                                                                  \
            ST cur = xxc__a_load_##B(p);                                                   \
            for (;;)                                                                       \
            {                                                                              \
                ST seen = xxc__a_cas_##B(p, cur, (ST)(cur - v));                           \
                if (seen == cur) return cur;                                               \
                cur = seen;                                                                \
            }                                                                              \
        }                                                                                  \
        static inline ST xxc__a_and_##B(volatile ST *p, ST v)                              \
        {                                                                                  \
            ST cur = xxc__a_load_##B(p);                                                   \
            for (;;)                                                                       \
            {                                                                              \
                ST seen = xxc__a_cas_##B(p, cur, (ST)(cur & v));                           \
                if (seen == cur) return cur;                                               \
                cur = seen;                                                                \
            }                                                                              \
        }                                                                                  \
        static inline ST xxc__a_or_##B(volatile ST *p, ST v)                               \
        {                                                                                  \
            ST cur = xxc__a_load_##B(p);                                                   \
            for (;;)                                                                       \
            {                                                                              \
                ST seen = xxc__a_cas_##B(p, cur, (ST)(cur | v));                           \
                if (seen == cur) return cur;                                               \
                cur = seen;                                                                \
            }                                                                              \
        }                                                                                  \
        static inline ST xxc__a_xor_##B(volatile ST *p, ST v)                              \
        {                                                                                  \
            ST cur = xxc__a_load_##B(p);                                                   \
            for (;;)                                                                       \
            {                                                                              \
                ST seen = xxc__a_cas_##B(p, cur, (ST)(cur ^ v));                           \
                if (seen == cur) return cur;                                               \
                cur = seen;                                                                \
            }                                                                              \
        }

    #if defined(_M_X64) || defined(_M_IX86) || defined(__x86_64__) || defined(__i386__)
        #define XXC_MSVC_LOAD_BODY(B, ST) \
            ST v = *p; _ReadWriteBarrier(); return v;
    #else
        #define XXC_MSVC_LOAD_BODY(B, ST) \
            return xxc__a_cas_##B(p, (ST)0, (ST)0);
    #endif

    XXC_MSVC_ATOMIC_WIDTH(8,  char,    _InterlockedCompareExchange8)
    XXC_MSVC_ATOMIC_WIDTH(16, short,   _InterlockedCompareExchange16)
    XXC_MSVC_ATOMIC_WIDTH(32, long,    _InterlockedCompareExchange)
    XXC_MSVC_ATOMIC_WIDTH(64, __int64, _InterlockedCompareExchange64)

    #define XXC_CAT_INNER(a, b) a##b
    #define XXC_CAT(a, b)       XXC_CAT_INNER(a, b)

    #define XXC_A_LOAD(B, P, ORD)               XXC_CAT(xxc__a_load_, B)(P)
    #define XXC_A_STORE(B, P, V, ORD)           XXC_CAT(xxc__a_store_, B)((P), (V))
    #define XXC_A_XCHG(B, P, V, ORD)            XXC_CAT(xxc__a_xchg_, B)((P), (V))
    #define XXC_A_ADD(B, P, V, ORD)             XXC_CAT(xxc__a_add_, B)((P), (V))
    #define XXC_A_SUB(B, P, V, ORD)             XXC_CAT(xxc__a_sub_, B)((P), (V))
    #define XXC_A_AND(B, P, V, ORD)             XXC_CAT(xxc__a_and_, B)((P), (V))
    #define XXC_A_OR(B, P, V, ORD)              XXC_CAT(xxc__a_or_, B)((P), (V))
    #define XXC_A_XOR(B, P, V, ORD)             XXC_CAT(xxc__a_xor_, B)((P), (V))

    #define XXC_A_CAS(B, P, EP, D, WEAK, S, F)  XXC_CAT(xxc__a_cas_ex_, B)((P), (EP), (D))

    #define XXC_MSVC_CAS_HELPER(B, ST)                                                     \
        static inline int xxc__a_cas_ex_##B(volatile ST *p, ST *expected, ST desired)      \
        {                                                                                  \
            ST want = *expected;                                                           \
            ST seen = xxc__a_cas_##B(p, want, desired);                                    \
            if (seen == want) return 1;                                                    \
            *expected = seen;                                                              \
            return 0;                                                                      \
        }
    XXC_MSVC_CAS_HELPER(8,  char)
    XXC_MSVC_CAS_HELPER(16, short)
    XXC_MSVC_CAS_HELPER(32, long)
    XXC_MSVC_CAS_HELPER(64, __int64)

    static inline void atomic_fence(atomic_order_t order)
    {
        _ReadWriteBarrier();
    #if defined(_M_X64) || defined(_M_IX86) || defined(__x86_64__) || defined(__i386__)
        if (order == ATOMIC_SEQ_CST) _mm_mfence();
    #else
        (void)order;
        __dmb(0xB);
    #endif
    }

    static inline void cpu_relax(void)
    {
    #if defined(_M_X64) || defined(_M_IX86) || defined(__x86_64__) || defined(__i386__)
        _mm_pause();
    #else
        __yield();
    #endif
    }

#endif

#define ATOMIC_INIT(v) { (v) }

#define XXC_ATOMIC_DEFINE_INT(NAME, T, B, ST)                                                        \
    typedef struct { XXC_ALIGNAS(sizeof(T)) volatile T value; } atomic_##NAME##_t;                   \
                                                                                                     \
    static inline void atomic_##NAME##_init(volatile atomic_##NAME##_t *a, T v) { a->value = v; }     \
                                                                                                     \
    static inline T atomic_##NAME##_load_explicit(const volatile atomic_##NAME##_t *a, atomic_order_t o) \
    { return (T)XXC_A_LOAD(B, (volatile ST *)&a->value, o); }                                        \
    static inline T atomic_##NAME##_load(const volatile atomic_##NAME##_t *a)                         \
    { return atomic_##NAME##_load_explicit(a, ATOMIC_SEQ_CST); }                                     \
                                                                                                     \
    static inline void atomic_##NAME##_store_explicit(volatile atomic_##NAME##_t *a, T v, atomic_order_t o) \
    { XXC_A_STORE(B, (volatile ST *)&a->value, (ST)v, o); }                                          \
    static inline void atomic_##NAME##_store(volatile atomic_##NAME##_t *a, T v)                      \
    { atomic_##NAME##_store_explicit(a, v, ATOMIC_SEQ_CST); }                                        \
                                                                                                     \
    static inline T atomic_##NAME##_exchange_explicit(volatile atomic_##NAME##_t *a, T v, atomic_order_t o) \
    { return (T)XXC_A_XCHG(B, (volatile ST *)&a->value, (ST)v, o); }                                 \
    static inline T atomic_##NAME##_exchange(volatile atomic_##NAME##_t *a, T v)                      \
    { return atomic_##NAME##_exchange_explicit(a, v, ATOMIC_SEQ_CST); }                              \
                                                                                                     \
    static inline bool atomic_##NAME##_cas_explicit(volatile atomic_##NAME##_t *a, T *expected, T desired,    \
                                                    atomic_order_t succ, atomic_order_t fail)        \
    {                                                                                                \
        ST e = (ST)*expected;                                                                        \
        bool ok = XXC_A_CAS(B, (volatile ST *)&a->value, &e, (ST)desired, 0, succ, fail) != 0;       \
        *expected = (T)e;                                                                            \
        return ok;                                                                                   \
    }                                                                                                \
    static inline bool atomic_##NAME##_cas(volatile atomic_##NAME##_t *a, T *expected, T desired)     \
    { return atomic_##NAME##_cas_explicit(a, expected, desired, ATOMIC_SEQ_CST, ATOMIC_SEQ_CST); }   \
                                                                                                     \
    static inline bool atomic_##NAME##_cas_weak_explicit(volatile atomic_##NAME##_t *a, T *expected,          \
                                        T desired, atomic_order_t succ, atomic_order_t fail)         \
    {                                                                                                \
        ST e = (ST)*expected;                                                                        \
        bool ok = XXC_A_CAS(B, (volatile ST *)&a->value, &e, (ST)desired, 1, succ, fail) != 0;       \
        *expected = (T)e;                                                                            \
        return ok;                                                                                   \
    }                                                                                                \
    static inline bool atomic_##NAME##_cas_weak(volatile atomic_##NAME##_t *a, T *expected, T desired)        \
    { return atomic_##NAME##_cas_weak_explicit(a, expected, desired, ATOMIC_SEQ_CST, ATOMIC_SEQ_CST); }

#define XXC_ATOMIC_DEFINE_ARITH(NAME, T, B, ST)                                                      \
    static inline T atomic_##NAME##_fetch_add_explicit(volatile atomic_##NAME##_t *a, T v, atomic_order_t o)  \
    { return (T)XXC_A_ADD(B, (volatile ST *)&a->value, (ST)v, o); }                                  \
    static inline T atomic_##NAME##_fetch_add(volatile atomic_##NAME##_t *a, T v)                             \
    { return atomic_##NAME##_fetch_add_explicit(a, v, ATOMIC_SEQ_CST); }                             \
    static inline T atomic_##NAME##_fetch_sub_explicit(volatile atomic_##NAME##_t *a, T v, atomic_order_t o)  \
    { return (T)XXC_A_SUB(B, (volatile ST *)&a->value, (ST)v, o); }                                  \
    static inline T atomic_##NAME##_fetch_sub(volatile atomic_##NAME##_t *a, T v)                             \
    { return atomic_##NAME##_fetch_sub_explicit(a, v, ATOMIC_SEQ_CST); }                             \
    static inline T atomic_##NAME##_fetch_and_explicit(volatile atomic_##NAME##_t *a, T v, atomic_order_t o)  \
    { return (T)XXC_A_AND(B, (volatile ST *)&a->value, (ST)v, o); }                                  \
    static inline T atomic_##NAME##_fetch_and(volatile atomic_##NAME##_t *a, T v)                             \
    { return atomic_##NAME##_fetch_and_explicit(a, v, ATOMIC_SEQ_CST); }                             \
    static inline T atomic_##NAME##_fetch_or_explicit(volatile atomic_##NAME##_t *a, T v, atomic_order_t o)   \
    { return (T)XXC_A_OR(B, (volatile ST *)&a->value, (ST)v, o); }                                   \
    static inline T atomic_##NAME##_fetch_or(volatile atomic_##NAME##_t *a, T v)                              \
    { return atomic_##NAME##_fetch_or_explicit(a, v, ATOMIC_SEQ_CST); }                               \
    static inline T atomic_##NAME##_fetch_xor_explicit(volatile atomic_##NAME##_t *a, T v, atomic_order_t o)  \
    { return (T)XXC_A_XOR(B, (volatile ST *)&a->value, (ST)v, o); }                                  \
    static inline T atomic_##NAME##_fetch_xor(volatile atomic_##NAME##_t *a, T v)                             \
    { return atomic_##NAME##_fetch_xor_explicit(a, v, ATOMIC_SEQ_CST); }

#if defined(XXC_ATOMIC_GNU)
    #define XXC_ST_8  u8
    #define XXC_ST_16 u16
    #define XXC_ST_32 u32
    #define XXC_ST_64 u64
#else
    #define XXC_ST_8  char
    #define XXC_ST_16 short
    #define XXC_ST_32 long
    #define XXC_ST_64 __int64
#endif

XXC_ATOMIC_DEFINE_INT(u8,  u8,  8,  XXC_ST_8)
XXC_ATOMIC_DEFINE_INT(i8,  i8,  8,  XXC_ST_8)
XXC_ATOMIC_DEFINE_INT(u16, u16, 16, XXC_ST_16)
XXC_ATOMIC_DEFINE_INT(i16, i16, 16, XXC_ST_16)
XXC_ATOMIC_DEFINE_INT(u32, u32, 32, XXC_ST_32)
XXC_ATOMIC_DEFINE_INT(i32, i32, 32, XXC_ST_32)
XXC_ATOMIC_DEFINE_INT(u64, u64, 64, XXC_ST_64)
XXC_ATOMIC_DEFINE_INT(i64, i64, 64, XXC_ST_64)

XXC_ATOMIC_DEFINE_ARITH(u8,  u8,  8,  XXC_ST_8)
XXC_ATOMIC_DEFINE_ARITH(i8,  i8,  8,  XXC_ST_8)
XXC_ATOMIC_DEFINE_ARITH(u16, u16, 16, XXC_ST_16)
XXC_ATOMIC_DEFINE_ARITH(i16, i16, 16, XXC_ST_16)
XXC_ATOMIC_DEFINE_ARITH(u32, u32, 32, XXC_ST_32)
XXC_ATOMIC_DEFINE_ARITH(i32, i32, 32, XXC_ST_32)
XXC_ATOMIC_DEFINE_ARITH(u64, u64, 64, XXC_ST_64)
XXC_ATOMIC_DEFINE_ARITH(i64, i64, 64, XXC_ST_64)

#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
    #define XXC_PTR_BITS 64
    #define XXC_ST_PTR   XXC_ST_64
#else
    #define XXC_PTR_BITS 32
    #define XXC_ST_PTR   XXC_ST_32
#endif

XXC_ATOMIC_DEFINE_INT(size, size_t, XXC_PTR_BITS, XXC_ST_PTR)
XXC_ATOMIC_DEFINE_ARITH(size, size_t, XXC_PTR_BITS, XXC_ST_PTR)

typedef struct { 
    XXC_ALIGNAS(1) volatile u8 value; 
} atomic_bool_t;

static inline void atomic_bool_init(volatile atomic_bool_t *a, bool v) 
{ 
    a->value = v ? 1 : 0; 
}

static inline bool atomic_bool_load_explicit(const volatile atomic_bool_t *a, atomic_order_t o)
{ 
    return XXC_A_LOAD(8, (volatile XXC_ST_8 *)&a->value, o) != 0; 
}

static inline bool atomic_bool_load(const volatile atomic_bool_t *a)
{ 
    return atomic_bool_load_explicit(a, ATOMIC_SEQ_CST); 
}

static inline void atomic_bool_store_explicit(volatile atomic_bool_t *a, bool v, atomic_order_t o)
{ 
    XXC_A_STORE(8, (volatile XXC_ST_8 *)&a->value, (XXC_ST_8)(v ? 1 : 0), o); 
}

static inline void atomic_bool_store(volatile atomic_bool_t *a, bool v)
{ 
    atomic_bool_store_explicit(a, v, ATOMIC_SEQ_CST); 
}

static inline bool atomic_bool_exchange_explicit(volatile atomic_bool_t *a, bool v, atomic_order_t o)
{ 
    return XXC_A_XCHG(8, (volatile XXC_ST_8 *)&a->value, (XXC_ST_8)(v ? 1 : 0), o) != 0; 
}

static inline bool atomic_bool_exchange(volatile atomic_bool_t *a, bool v)
{ 
    return atomic_bool_exchange_explicit(a, v, ATOMIC_SEQ_CST); 
}

static inline bool atomic_bool_cas_explicit(volatile atomic_bool_t *a, bool *expected, bool desired,
                                            atomic_order_t succ, atomic_order_t fail)
{
    XXC_ST_8 e = (XXC_ST_8)(*expected ? 1 : 0);
    bool ok = XXC_A_CAS(8, (volatile XXC_ST_8 *)&a->value, &e, (XXC_ST_8)(desired ? 1 : 0), 0, succ, fail) != 0;
    *expected = e != 0;
    return ok;
}
static inline bool atomic_bool_cas(volatile atomic_bool_t *a, bool *expected, bool desired)
{ 
    return atomic_bool_cas_explicit(a, expected, desired, ATOMIC_SEQ_CST, ATOMIC_SEQ_CST); 
}

typedef struct { 
    XXC_ALIGNAS(sizeof(void *)) void *volatile value; 
} atomic_ptr_t;

#if defined(XXC_ATOMIC_GNU)
    #define XXC_PTR_CAST_IN(p)   (p)
    #define XXC_PTR_STORE_T      void *
#else
    #define XXC_PTR_CAST_IN(p)   ((XXC_ST_PTR)(p))
    #define XXC_PTR_STORE_T      XXC_ST_PTR
#endif

static inline void atomic_ptr_init(volatile atomic_ptr_t *a, void *v) { a->value = v; }

static inline void *atomic_ptr_load_explicit(const volatile atomic_ptr_t *a, atomic_order_t o)
{ 
    return (void *)XXC_A_LOAD(XXC_PTR_BITS, (XXC_PTR_STORE_T volatile *)&a->value, o); 
}

static inline void *atomic_ptr_load(const volatile atomic_ptr_t *a)
{ 
    return atomic_ptr_load_explicit(a, ATOMIC_SEQ_CST); 
}

static inline void atomic_ptr_store_explicit(volatile atomic_ptr_t *a, void *v, atomic_order_t o)
{ 
    XXC_A_STORE(XXC_PTR_BITS, (XXC_PTR_STORE_T volatile *)&a->value, (XXC_PTR_STORE_T)XXC_PTR_CAST_IN(v), o); 
}
static inline void atomic_ptr_store(volatile atomic_ptr_t *a, void *v)
{ 
    atomic_ptr_store_explicit(a, v, ATOMIC_SEQ_CST); 
}

static inline void *atomic_ptr_exchange_explicit(volatile atomic_ptr_t *a, void *v, atomic_order_t o)
{ 
    return (void *)XXC_A_XCHG(XXC_PTR_BITS, (XXC_PTR_STORE_T volatile *)&a->value, (XXC_PTR_STORE_T)XXC_PTR_CAST_IN(v), o); 
}

static inline void *atomic_ptr_exchange(volatile atomic_ptr_t *a, void *v)
{ 
    return atomic_ptr_exchange_explicit(a, v, ATOMIC_SEQ_CST); 
}

static inline bool atomic_ptr_cas_explicit(volatile atomic_ptr_t *a, void **expected, void *desired,
                                           atomic_order_t succ, atomic_order_t fail)
{
    XXC_PTR_STORE_T e = (XXC_PTR_STORE_T)XXC_PTR_CAST_IN(*expected);
    bool ok = XXC_A_CAS(XXC_PTR_BITS, (XXC_PTR_STORE_T volatile *)&a->value, &e,
                        (XXC_PTR_STORE_T)XXC_PTR_CAST_IN(desired), 0, succ, fail) != 0;
    *expected = (void *)e;
    return ok;
}

static inline bool atomic_ptr_cas(volatile atomic_ptr_t *a, void **expected, void *desired)
{ 
    return atomic_ptr_cas_explicit(a, expected, desired, ATOMIC_SEQ_CST, ATOMIC_SEQ_CST); 
}

static inline bool atomic_ptr_cas_weak_explicit(volatile atomic_ptr_t *a, void **expected, void *desired,
                                                atomic_order_t succ, atomic_order_t fail)
{
    XXC_PTR_STORE_T e = (XXC_PTR_STORE_T)XXC_PTR_CAST_IN(*expected);
    bool ok = XXC_A_CAS(XXC_PTR_BITS, (XXC_PTR_STORE_T volatile *)&a->value, &e,
                        (XXC_PTR_STORE_T)XXC_PTR_CAST_IN(desired), 1, succ, fail) != 0;
    *expected = (void *)e;
    return ok;
}

static inline bool atomic_ptr_cas_weak(volatile atomic_ptr_t *a, void **expected, void *desired)
{ 
    return atomic_ptr_cas_weak_explicit(a, expected, desired, ATOMIC_SEQ_CST, ATOMIC_SEQ_CST); 
}

#if defined(__cplusplus)
    }
#endif

#endif /* ATOMIC_H */