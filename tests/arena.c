/*
For license details see ../../LICENSE
*/

/**
 * \file arena.c
 * \date Aug 21, 2017
 */

#include "defs.h"

#include "ax/arena.h"

static
void basic(void)
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

setupSuite(arena)
{
    addTest(basic);
}
