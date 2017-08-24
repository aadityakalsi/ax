/*
For license details see ../../LICENSE
*/

/**
 * \file arena.c
 * \date Aug 20, 2017
 */

#include "ax/arena.h"
#include "ax/assert.h"

#include <uv.h>

#include <stdlib.h>

static
ax_arena_page_t* create_arena_page(ax_sz pg_sz, ax_sz align)
{
    ax_sz hdr_sz = sizeof(ax_arena_page_t);
    ax_arena_page_t* pg;
    ax_sz shift;
    AX_ASSERT_MSG((align & (align-1)) == 0, "Alignment must be a power of 2.");
    AX_ASSERT_MSG((pg_sz & (align - 1)) == 0, "Arena page size should be aligned");
    hdr_sz = (hdr_sz + align - 1) & ~(align - 1);
    pg = (ax_arena_page_t*)malloc(pg_sz + hdr_sz + align);
    shift = (((ax_sz)pg) & (align - 1));
    if (shift) {
        pg = (ax_arena_page_t*)((ax_u8*)(pg) + shift);
    }
    if (pg) {
        pg->curr = ((ax_u8*)pg) + hdr_sz;
        pg->end = pg->curr + pg_sz;
        pg->prev = AX_NULL;
        pg->shift = shift;
    }
    return pg;
}

static
void destroy_arena_page(ax_arena_page_t* pg)
{
    free(((ax_u8*)pg) - pg->shift);
}

int ax_arena_init(ax_arena_t* ar, ax_sz min_pg_sz, ax_sz align)
{
    ax_arena_page_t* pg;
    AX_ASSERT_MSG((min_pg_sz & (align - 1)) == 0, "Arena min page size should be aligned");
    pg = create_arena_page(min_pg_sz, align);
    if (!pg) {
        return AX_ENOMEM;
    }
    ar->curr = pg;
    ar->min_page_size = min_pg_sz;
    ar->align = align;
    return 0;
}

void* ax_arena_alloc(ax_arena_t* ar, ax_sz sz)
{
    ax_arena_page_t* curr;
    void* mem;

    AX_ASSERT(ar);
    AX_ASSERT(ar->curr);

    curr = ar->curr;
    sz = (sz + ar->align - 1) & ~(ar->align - 1);
    if (curr->curr + sz <= curr->end) {
        mem = curr->curr;
        curr->curr += sz;
    } else {
        ax_arena_page_t* newpg = create_arena_page(sz > ar->min_page_size ? sz : ar->min_page_size, ar->align);
        if (!newpg) {
            mem = AX_NULL;
        } else {
            AX_ASSERT(newpg->curr + sz <= newpg->end);
            mem = newpg->curr;
            newpg->curr += sz;
            newpg->prev = curr;
            ar->curr = newpg;
        }
    }
    return mem;
}

int ax_arena_destroy(ax_arena_t* ar)
{
    ax_arena_page_t* pg = ar->curr;
    while (pg != AX_NULL) {
        ax_arena_page_t* prev = pg->prev;
        destroy_arena_page(pg);
        pg = prev;
    }
    ar->curr = AX_NULL;
    return 0;
}
