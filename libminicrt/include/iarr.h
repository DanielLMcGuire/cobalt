// ++C C Runtime Library (libminicrt)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#ifndef IARR_H
#define IARR_H

#include <def.h>

typedef struct {
    int *data;
    size_t len;
    size_t cap;
} iarr_t;

iarr_t iarr_new(const int *init, size_t init_len);
void    iarr_append(iarr_t *arr, const int *append, size_t append_len);
void    iarr_append_val(iarr_t *arr, int val);
void    iarr_free(iarr_t *arr);

#endif /* IARR_H */