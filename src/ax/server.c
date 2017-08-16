/*
For license details see ../../LICENSE
*/

/**
 * \file server.c
 * \date Aug 11, 2017
 */

#include "ax/server.h"
#include "ax/assert.h"

#include <uv.h>

AX_STRUCT_TYPE(ax_server_impl_t)
{
    uv_loop_t loop;
};

AX_STATIC_ASSERT(sizeof(ax_server_t) >= sizeof(ax_server_impl_t), type_too_small);
