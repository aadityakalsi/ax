/*
For license details see ../../LICENSE
*/

/**
 * \file assert.h
 * \date Aug 11, 2017
 */

#ifndef _AX_ASSERT_H_
#define _AX_ASSERT_H_

#include "ax/types.h"

#define ____CONCAT_(a, b) a##b
#define ____CONCAT(a, b) ____CONCAT_(a, b)

#define AX_STATIC_ASSERT(e, m) char ____CONCAT(m, _assert)[e ? 1 : -1]


#define __STR_(x) #x
#define __STR(x) __STR_(x)

#define AX_INVARIANT(e) (!!(e)) ? 0 : _axAssertFailed("(" __FILE__ ":" __STR(__LINE__) ") " __STR(e), "")
#define AX_INVARIANT_MSG(e, m) (!!(e)) ? 0 : _axAssertFailed("(" __FILE__ ":" __STR(__LINE__) ") " __STR(e), m)

#ifndef NDEBUG
#  define AX_ASSERT(e) AX_INVARIANT(e)
#  define AX_ASSERT_MSG(e, m) AX_INVARIANT_MSG(e, m)
#else
#  define AX_ASSERT(e) ((void)0)
#  define AX_ASSERT_MSG(e, m) ((void)0)
#endif

AX_API
void _axAssertFailed(ax_const_str hdr, ax_const_str msg);

#endif/*_AX_ASSERT_H_*/
