// ++C CRT
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#ifndef DSTR_H
#define DSTR_H

#include <def.h>

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} dstr_t;

dstr_t dstr_new(const char *init);
void   dstr_append(dstr_t *s, const char *append);
void   dstr_append_char(dstr_t *s, char append);
void dstr_append_int(dstr_t *s, int val);
void dstr_append_size_t(dstr_t *s, size_t val);
void   dstr_free(dstr_t *s);

#endif /* DSTR_H */