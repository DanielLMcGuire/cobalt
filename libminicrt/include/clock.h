// ++C C Runtime Library (libminicrt)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#ifndef CLOCK_H
#define CLOCK_H

#include <def.h>

#ifdef __cplusplus
extern "C" {
#endif

u64 clock_monotonic_ns(void);
u64 clock_monotonic_ms(void);

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_H */
