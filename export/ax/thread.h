/*
For license details see ../../LICENSE
*/

/**
 * \file thread.h
 * \date Aug 16, 2017
 */

#ifndef _AX_THREAD_H_
#define _AX_THREAD_H_

#include "ax/types.h"

AX_HIDDEN_TYPE(ax_thread_t, 16);

typedef void (*ax_thread_cb)(void* arg);

AX_API
int ax_thread_create(ax_thread_t* tid, ax_thread_cb cbk, void* arg);

AX_API
ax_thread_t ax_thread_self(void);

AX_API
int ax_thread_join(ax_thread_t* tid);

AX_API
int ax_thread_equal(ax_thread_t const* tid0, ax_thread_t const* tid1);

#endif/*_AX_THREAD_H_*/
