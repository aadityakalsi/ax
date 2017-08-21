/*
For license details see ../../LICENSE
*/

/**
 * \file hashtable.h
 * \date Aug 20, 2017
 */

#ifndef _AX_HASHTABLE_H_
#define _AX_HASHTABLE_H_

#include "ax/types.h"
#include "ax/arena.h"

AX_STRUCT_TYPE(ax_ht_elem_t)
{
    ax_ht_elem_t* next;
    void* key;
    void* value;
    ax_sz hash;
};

AX_STRUCT_TYPE(ax_ht_entry_t)
{
    void const* const key;
    void* value;
};

typedef int (*ax_ht_eq_fn)(void const* k0, void const* k1);
typedef ax_sz (*ax_ht_hash_fn)(void const* k);
typedef void (*ax_ht_delete_fn)(void* key, void* value);

AX_STRUCT_TYPE(ax_ht_t)
{
    ax_ht_elem_t** buckets;
    ax_sz size;
    ax_sz cap;
    ax_flt load_fac;
    ax_ht_hash_fn hash_fn;
    ax_ht_eq_fn eq_fn;
    ax_ht_delete_fn del_fn;
    ax_arena_t arena;
};

AX_API
int ax_ht_init(ax_ht_t* ht, ax_sz init_size, ax_flt load_fac, ax_ht_hash_fn hash, ax_ht_eq_fn eq, ax_ht_delete_fn del);

AX_API
int ax_ht_init_default(ax_ht_t* ht, ax_ht_hash_fn hash, ax_ht_eq_fn eq, ax_ht_delete_fn del);

AX_API
ax_ht_entry_t* ax_ht_find(ax_ht_t const* ht, void const* key);

AX_API
ax_ht_entry_t* ax_ht_insert(ax_ht_t* ht, void const* key, void const* value, int* ins);

AX_API
void ax_ht_erase(ax_ht_t* ht, void const* key);

AX_API
int ax_ht_destroy(ax_ht_t* ht);

#endif/*_AX_HASHTABLE_H_*/
