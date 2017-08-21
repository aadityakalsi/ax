/*
For license details see ../../LICENSE
*/

/**
 * \file strtable.h
 * \date Aug 21, 2017
 */

#ifndef _AX_STRTABLE_H_
#define _AX_STRTABLE_H_

#include "ax/hashtable.h"

typedef ax_ht_t ax_st_t;

AX_STRUCT_TYPE(ax_st_pair_t)
{
    ax_const_str const key;
    ax_str value;
};

typedef void (*ax_st_visit_fn)(ax_const_str, ax_str);

AX_API
int ax_st_init(ax_st_t* st, ax_sz init_size, ax_flt load_fac);

AX_API
int ax_st_init_default(ax_st_t* st);

AX_API
ax_st_pair_t* ax_st_find(ax_st_t const* st, ax_const_str key);

AX_API
ax_st_pair_t* ax_st_insert(ax_st_t* st, ax_const_str key, ax_str value, int* ins);

AX_API
ax_str ax_st_create_str(ax_st_t* st, ax_const_str copy_from);

AX_API
ax_sz  ax_st_str_capacity(ax_st_t const* st, ax_const_str s);

AX_API
void ax_st_foreach(ax_st_t const* st, ax_st_visit_fn fn);

AX_API
int ax_st_erase(ax_st_t* st, ax_const_str key);

AX_API
int ax_st_destroy(ax_st_t* st);

#endif/*_AX_STRTABLE_H_*/
