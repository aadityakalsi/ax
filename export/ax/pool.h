/*
For license details see ../../LICENSE
*/

/**
 * \file pool.h
 * \date Aug 22, 2017
 */

#ifndef _AX_POOL_H_
#define _AX_POOL_H_

#include "ax/types.h"
#include "ax/assert.h"
#include "ax/arena.h"

AX_STRUCT_TYPE(ax_pool_t)
{
    ax_arena_t arena;
    void* head;
    ax_sz elem_size;
};

inline static int ax_pool_init(ax_pool_t* p, ax_sz nelem_size)
{
    int err;
    AX_ASSERT(nelem_size >= sizeof(void*));
    err = ax_arena_init_default(&p->arena);
    if (!err) {
        p->head = AX_NULL;
        p->elem_size = nelem_size;
    }
    return err;
}

inline static void* ax_pool_alloc(ax_pool_t* p)
{
    void* mem;
    if (p->head == AX_NULL) {
        mem = ax_arena_alloc(&p->arena, p->elem_size);
    } else {
        mem = p->head;
        p->head = *(void**)mem;
    }
    return mem;
}

inline static void ax_pool_free(ax_pool_t* p, void* ptr)
{
    if (ptr) {
        *(void**)ptr = p->head;
        p->head = ptr;
    }
}

inline static int ax_pool_destroy(ax_pool_t* p)
{
    p->head = AX_NULL;
    return ax_arena_destroy(&p->arena);
}

#endif/*_AX_POOL_H_*/