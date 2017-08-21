/*
For license details see ../../LICENSE
*/

/**
 * \file hashtable.c
 * \date Aug 21, 2017
 */

#include "ax/hashtable.h"
#include "ax/assert.h"

#include <uv.h>

#include <stdlib.h>

static
ax_ht_elem_t** create_buckets(ax_sz sz)
{
    // AX_ASSERT_MSG((sz & (sz + 1)) == 0, "Buckets must be pow(2)-1");
    return (ax_ht_elem_t**)(sz ? calloc(sz, sizeof(ax_ht_elem_t*)) : AX_NULL);
}

static
void destroy_buckets(ax_ht_elem_t** p)
{
    free(p);
}

static
ax_u64 next_pow_2(ax_u64 v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v ? v : 1;
}

static
int rehash(ax_ht_t* ht)
{
    ax_flt div = (ht->size + 1) / ht->load_fac;
    ax_sz newcap;
    ax_ht_elem_t** buckets;
    ax_ht_elem_t* el;
    ax_ht_elem_t* next;
    ax_sz idx;
    
    div = ceilf(div);
    newcap = next_pow_2((ax_sz)div) - 1;
    newcap = newcap < 7 ? 7 : newcap;
    if (newcap < ht->cap) {
        return UV__ENOMEM;
    }

    buckets = create_buckets(newcap);
    if (!buckets) {
        return UV__ENOMEM;
    }

    for (ax_sz i = 0; i < ht->cap; ++i) {
        el = ht->buckets[i];
        ht->buckets[i] = AX_NULL;
        while (el) {
            next = el->next;
            idx = el->hash % newcap;
            el->next = buckets[idx];
            buckets[idx] = el;
            el = next;
        }
    }

    destroy_buckets(ht->buckets);
    ht->buckets = buckets;
    ht->cap = newcap;

    return 0;
}

int ax_ht_init(ax_ht_t* ht, ax_sz init_size, ax_flt load_fac, ax_ht_hash_fn hash, ax_ht_eq_fn eq, ax_ht_delete_fn del)
{
    ax_ht_elem_t** buckets = create_buckets(init_size);

    if ((!buckets && init_size != 0) || ax_arena_init_default(&ht->arena)) {
        return UV__ENOMEM;
    }

    ht->buckets = buckets;
    ht->size = 0;
    ht->cap = init_size;
    ht->load_fac = load_fac;
    ht->hash_fn = hash;
    ht->eq_fn = eq;
    ht->del_fn = del;

    return 0;
}

int ax_ht_init_default(ax_ht_t* ht, ax_ht_hash_fn hash, ax_ht_eq_fn eq, ax_ht_delete_fn del)
{
    return ax_ht_init(ht, 0, (ax_flt)0.618, hash, eq, del);
}

ax_ht_entry_t* ax_ht_find(ax_ht_t const* ht, void const* key)
{
    ax_sz h = ht->hash_fn(key);
    ax_ht_entry_t* ret = AX_NULL;
    ax_sz h_mod;
    ax_ht_elem_t* el;

    if (ht->buckets) {
        h_mod = h % (ht->cap);
        el = ht->buckets[h_mod];
        while (el) {
            if (el->key == key || ht->eq_fn(el->key, key)) {
                ret = (ax_ht_entry_t*)(&el->key);
                break;
            }
            el = el->next;
        }
    }

    return ret;
}

ax_ht_entry_t* ax_ht_insert(ax_ht_t* ht, void const* key, void const* value, int* ins)
{
    ax_sz h = ht->hash_fn(key);
    ax_ht_entry_t* ret = AX_NULL;
    ax_sz h_mod;
    ax_ht_elem_t* el;
    
    if (ht->buckets) {
        h_mod = h % (ht->cap);
        el = ht->buckets[h_mod];
        while (el) {
            if (el->key == key || ht->eq_fn(el->key, key)) {
                ret = (ax_ht_entry_t*)(&el->key);
                ins ? (*ins = 0, 0) : 0;
                break;
            }
            el = el->next;
        }
    }

    if (!ret) {
        if ((ht->load_fac * ht->cap) < (ht->size + 1)) {
            if (rehash(ht) == 0) {
                AX_ASSERT((ht->load_fac * ht->cap) >= (ht->size + 1));
                ret = ax_ht_insert(ht, key, value, ins);                    
            }
        } else {
            ax_ht_elem_t* newel = (ax_ht_elem_t*)ax_arena_alloc(&ht->arena, sizeof(ax_ht_elem_t));
            if (newel) {
                newel->next = ht->buckets[h_mod];
                newel->key = (void*)key;
                newel->value = (void*)value;
                newel->hash = h;
                ht->buckets[h_mod] = newel;
                ht->size++;
                ret = (ax_ht_entry_t*)&newel->key;
                ins ? (*ins = 1, 0) : 0;
            }
        }
    }

    return ret;
}

void ax_ht_erase(ax_ht_t* ht, void const* key);

int ax_ht_destroy(ax_ht_t* ht)
{
    for (ax_sz i = 0; i < ht->cap; ++i) {
        ax_ht_elem_t* el = ht->buckets[i];
        while (el) {
            ht->del_fn(el->key, el->value);
            el = el->next;
        }
    }
    destroy_buckets(ht->buckets);
    ht->buckets = AX_NULL;
    ht->size = 0;
    ht->cap = 0;
    return ax_arena_destroy(&ht->arena);
}
