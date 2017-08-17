/*
For license details see ../../LICENSE
*/

/**
 * \file types.c
 * \date Aug 16, 2017
 */

#include "ax/types.h"

#if  defined(_WIN32) && !(defined(__MINGW32__) || defined(__MINGW64__))
#include <intrin.h>
void _ReadWriteBarrier();
#pragma intrinsic(_ReadWriteBarrier);
#pragma intrinsic(_InterlockedCompareExchange);
#pragma intrinsic(_InterlockedExchangeAdd);
#define acq_barrier() _ReadWriteBarrier()
#define rel_barrier() _ReadWriteBarrier()
#else
#define acq_barrier() __asm__ __volatile__("": : :"memory")
#define rel_barrier() __asm__ __volatile__("": : :"memory")
#endif

ax_u32 ax_aload_u32(ax_u32* p)
{
    acq_barrier();
    return *p;
}

void ax_astore_u32(ax_u32* p, ax_u32 val)
{
    *p = val;
    rel_barrier();
}
