/*
For license details see ../../LICENSE
*/

/**
 * \file mem.c
 * \date Aug 21, 2017
 */

#include "defs.h"

#include "ax/arena.h"
#include "ax/pool.h"

static
void arena(void)
{
    ax_arena_t ar;
    ax_u8* curr;
    testThat(ax_arena_init(&ar, 128, 32) == 0);
    testThat(ar.curr != AX_NULL);
    testThat((((ax_sz)ar.curr) & 31) == 0);
    curr = ax_arena_alloc(&ar, 23);
    testThat(((ax_sz)curr & 31) == 0);
    testThat(ar.curr->curr == (curr + 32));
    curr = ax_arena_alloc(&ar, 256);
    testThat(((ax_sz)curr & 31) == 0);
    testThat(ar.curr->curr == ar.curr->end);
    testThat(ax_arena_destroy(&ar) == 0);
    testThat(ar.curr == AX_NULL);
}

static
void pool(void)
{
    ax_pool_t p;
    ax_u8* curr;
    ax_u8* prev;
    ax_u8* next;

    testThat(ax_pool_init(&p, 16) == 0);
    curr = (ax_u8*)p.arena.curr->curr;
    prev = ax_pool_alloc(&p);
    testThat(prev == curr);
    testThat(p.arena.curr->curr = (curr + 16));
    next = ax_pool_alloc(&p);
    testThat(next == (prev + 16));
    ax_pool_free(&p, AX_NULL);
    testThat(p.head == AX_NULL);
    ax_pool_free(&p, prev);
    testThat(p.head != AX_NULL);
    testThat(ax_pool_alloc(&p) == prev);
    testThat(ax_pool_destroy(&p) == 0);
    testThat(p.head == AX_NULL);
}

setupSuite(mem)
{
    addTest(arena);
    addTest(pool);
}
