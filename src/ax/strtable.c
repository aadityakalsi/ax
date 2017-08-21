/*
For license details see ../../LICENSE
*/

/**
 * \file strtable.c
 * \date Aug 21, 2017
 */

#include "ax/strtable.h"
#include "ax/assert.h"

#include <string.h>

static
ax_sz ax__st_hash(void const* str)
{
    ax_const_str s = (ax_const_str)str;
    ax_sz h = 0;
    while (*s) {
        h = ((h << 5) - h) + *s;
        ++s;
    }
    return h;
}

static
int ax__st_eq(void const* str0, void const* str1)
{
    return strcmp(str0, str1) == 0;
}

static
void ax__st_del(void* k, void* v)
{
}

int ax_st_init(ax_st_t* st, ax_sz init_size, ax_flt load_fac)
{
    /* align by 64 to allow string growth */
    return ax_ht_init(st, init_size, load_fac, ax__st_hash, ax__st_eq, ax__st_del, 32);
}

int ax_st_init_default(ax_st_t* st)
{
    return ax_st_init(st, 0, (ax_flt)0.618);
}

ax_st_pair_t* ax_st_find(ax_st_t const* st, ax_const_str key)
{
    return (ax_st_pair_t*)ax_ht_find(st, (void const*)key);
}

ax_str ax_st_create_str(ax_st_t* st, ax_const_str copy_from)
{
    ax_sz str_len = strlen(copy_from);
    ax_str s = (ax_str)ax_arena_alloc(&st->arena, str_len + 1);
    return s ? (ax_str)memcpy(s, copy_from, str_len + 1) : AX_NULL;
}

ax_sz  ax_st_str_capacity(ax_st_t const* st, ax_const_str s)
{
    ax_sz align = st->arena.align;
    ax_sz len = s ? strlen(s) : 0;
    return (len + align) & ~(align - 1);
}

ax_st_pair_t* ax_st_insert(ax_st_t* st, ax_const_str key, ax_str value, int* ins)
{
    int inserted = 0;
    ax_ht_entry_t* res = ax_ht_insert(st, (void const*)key, (void*)value, &inserted);
    if (inserted) {
        ax_str newkey = ax_st_create_str(st, key);
        ax_str newval = ax_st_create_str(st, value);
        if (newkey && newval) {
            *(void**)(&res->key) = newkey;
            res->value = newval;
        } else {
            int erased = ax_ht_erase(st, res->key);
            inserted = 0;
            res = AX_NULL;
            AX_ASSERT(erased == 1);
        }
    }
    ins ? (*ins = inserted, 0) : 0;
    return (ax_st_pair_t*)res;
}

void ax_st_foreach(ax_st_t const* st, ax_st_visit_fn fn)
{
    for (ax_sz i = 0; i < st->cap; ++i) {
        ax_ht_elem_t* el = st->buckets[i];
        while (el) {
            fn((ax_const_str)el->key, (ax_str)el->value);
            el = el->next;
        }
    }
}

int ax_st_erase(ax_st_t* st, ax_const_str key)
{
    return ax_ht_erase(st, (void const*)key);
}

int ax_st_destroy(ax_st_t* st)
{
    return ax_ht_destroy(st);
}
