// ++C C Runtime Library (libminicrt)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#ifndef PARR_H
#define PARR_H

#include <dstr.h>
#include <def.h>

typedef struct {
    void **data;
    size_t len;
    size_t cap;
} parr_t;

parr_t parr_new(const void **init, size_t init_len);

void parr_append(parr_t *arr, const void **append, size_t append_len);

void parr_append_val(parr_t *arr, void *val);

void parr_append_cstr(parr_t *arr, const char *str);

void parr_append_dstr(parr_t *arr, const dstr_t *ds);

parr_t parr_c_strings_to_dstr(const parr_t *csArr);

parr_t parr_dstr_to_c_strings(const parr_t *dsArr);

void parr_free_c_strings(parr_t *arr);

void parr_free_dstr(parr_t *arr);

void parr_free(parr_t *arr);

#endif /* PARR_H */