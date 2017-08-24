/*
For license details see ../../LICENSE
*/

/**
 * \file arena.h
 * \date Aug 20, 2017
 */

#ifndef _AX_ARENA_H_
#define _AX_ARENA_H_

#include "ax/types.h"

AX_STRUCT_TYPE(ax_arena_page_t)
{
    ax_u8* curr;
    ax_u8* end;
    ax_arena_page_t* prev;
    ax_sz shift;
};

AX_STRUCT_TYPE(ax_arena_t)
{
    ax_arena_page_t* curr;
    ax_sz min_page_size;
    ax_sz align;
};

AX_API
int ax_arena_init(ax_arena_t* ar, ax_sz min_pg_sz, ax_sz align);

AX_API
void* ax_arena_alloc(ax_arena_t* ar, ax_sz sz);

AX_API
int ax_arena_destroy(ax_arena_t* ar);

#endif/*_AX_ARENA_H_*/
