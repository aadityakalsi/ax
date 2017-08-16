/*
For license details see ../../LICENSE
*/

/**
 * \file thread.c
 * \date Aug 16, 2017
 */

#include "ax/assert.h"
#include "ax/thread.h"

#include <uv.h>

AX_STATIC_ASSERT(sizeof(ax_thread_t) >= sizeof(uv_thread_t), ax_thread_t_too_small);

#define to_uv(x) ((uv_thread_t*)(x))

int ax_thread_create(ax_thread_t* tid, ax_thread_cb cbk, void* arg)
{
    return uv_thread_create(to_uv(tid), cbk, arg);
}

ax_thread_t ax_thread_self(void)
{
    union {
        ax_thread_t ax;
        uv_thread_t uv;
    } t;
    memset(&t, 0, sizeof(t));
    t.uv = uv_thread_self();
    return t.ax;
}

int ax_thread_join(ax_thread_t* tid)
{
    return uv_thread_join(to_uv(tid));
}

int ax_thread_equal(ax_thread_t const* tid0, ax_thread_t const* tid1)
{
    return uv_thread_equal(to_uv(tid0), to_uv(tid1));
}
