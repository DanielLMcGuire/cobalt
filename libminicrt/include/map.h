// ++C C Runtime Library (libminicrt)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#ifndef MAP_H
#define MAP_H

#include <def.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    u64    hash;
    char  *key;
    size_t key_len;
    void  *value;
    bool   occupied;
} map_entry_t;

typedef struct {
    map_entry_t *entries;
    size_t       cap;
    size_t       len;
    void       (*value_free)(void *value);
} map_t;

map_t map_new(size_t initial_cap, void (*value_free)(void *value));
void  map_free(map_t *m);
void  map_clear(map_t *m);  

size_t map_len(const map_t *m);

bool map_set(map_t *m, const char *key, size_t key_len, void *value);
bool map_set_cstr(map_t *m, const char *key, void *value);

void *map_get(const map_t *m, const char *key, size_t key_len);
void *map_get_cstr(const map_t *m, const char *key);

bool map_contains(const map_t *m, const char *key, size_t key_len);
bool map_contains_cstr(const map_t *m, const char *key);

bool map_try_get(const map_t *m, const char *key, size_t key_len, void **out);
bool map_try_get_cstr(const map_t *m, const char *key, void **out);

bool map_remove(map_t *m, const char *key, size_t key_len);
bool map_remove_cstr(map_t *m, const char *key);

bool map_iterate(const map_t *m, size_t *cursor, const char **key_out, size_t *key_len_out, void **value_out);

u64 map_hash_bytes(const void *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* MAP_H */
