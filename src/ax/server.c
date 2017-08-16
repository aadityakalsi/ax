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

AX_STRUCT_TYPE(axServerImpl)
{
    uv_loop_t fLoop;
};

AX_STATIC_ASSERT(sizeof(axServer) >= sizeof(axServerImpl), type_too_small);
