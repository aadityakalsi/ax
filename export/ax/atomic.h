/*
For license details see ../../LICENSE
*/

/**
 * \file atomic.h
 * \date Aug 16, 2017
 */

#ifndef _AX_ATOMIC_H_
#define _AX_ATOMIC_H_

#include "ax/config.h"

typedef ax_i32 ax_atm_i32;
typedef void*  ax_atm_void_ptr;

#if !AX_MSVC && (AX_GCC || AX_CLANG)

inline static
ax_i32 ax_atomic_i32_load(ax_atm_i32* p)
{
#if defined(__i386__) || defined(__x86_64__)
    return *(ax_i32 volatile*)p;
#else
    return __sync_add_and_fetch(p, 0);
#endif
}

inline static
void ax_atomic_i32_store(ax_atm_i32* p, ax_i32 desired)
{
#if defined(__i386__) || defined(__x86_64__)
    __sync_synchronize();
    *(ax_i32 volatile*)p = desired;
#else
    __sync_lock_test_and_set(p, desired);
#endif
}

inline static
ax_i32 ax_atomic_i32_inc(ax_atm_i32* p)
{
    return (ax_i32)__sync_add_and_fetch(p, 1);
}

inline static
ax_i32 ax_atomic_i32_dec(ax_atm_i32* p)
{
    return (ax_i32)__sync_sub_and_fetch(p, 1);
}

inline static
ax_i32 ax_atomic_i32_add(ax_atm_i32* p, ax_i32 value)
{
    return (ax_i32)__sync_add_and_fetch(p, value);
}

inline static
ax_i32 ax_atomic_i32_sub(ax_atm_i32* p, ax_i32 value)
{
    return (ax_i32)__sync_sub_and_fetch(p, value);
}

inline static
ax_i32 ax_atomic_i32_xchg(ax_atm_i32* p, int desired)
{
    return (ax_i32)__sync_lock_test_and_set(p, desired);
}

inline static
void* ax_atomic_ptr_load(ax_atm_void_ptr* p)
{
#if defined(__i386__) || defined(__x86_64__)
    return *(void* volatile*)p;
#else
    return __sync_add_and_fetch(p, 0);
#endif
}

inline static
void ax_atomic_ptr_store(ax_atm_void_ptr* p, void* desired)
{
#if defined(__i386__) || defined(__x86_64__)
    __sync_synchronize();
    *(void* volatile*)p = desired;
#else
    __sync_lock_test_and_set(p, desired);
#endif
}

inline static
void* ax_atomic_ptr_xchg(ax_atm_void_ptr* p, void* desired)
{
    return (void*)__sync_lock_test_and_set(p, desired);
}

#else

#include <intrin.h>

#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedCompareExchangePointer)
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedExchangePointer)
#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedExchangeAdd)

inline static
ax_i32 ax_atomic_i32_load(ax_atm_i32* p)
{
    return _InterlockedExchangeAdd(p, 0);
}

inline static
void ax_atomic_i32_store(ax_atm_i32* p, ax_i32 desired)
{
    _InterlockedExchange(p, desired);
}

inline static
ax_i32 ax_atomic_i32_inc(ax_atm_i32* p)
{
    return _InterlockedIncrement(p);
}

inline static
ax_i32 ax_atomic_i32_dec(ax_atm_i32* p)
{
    return _InterlockedDecrement(p);
}

inline static
ax_i32 ax_atomic_i32_add(ax_atm_i32* p, ax_i32 value)
{
    return _InterlockedExchangeAdd(p, value) + value;
}

inline static
ax_i32 ax_atomic_i32_sub(ax_atm_i32* p, ax_i32 value)
{
    return _InterlockedExchangeAdd(p, -value) - value;
}

inline static
ax_i32 ax_atomic_i32_xchg(ax_atm_i32* p, ax_i32 desired)
{
    return _InterlockedExchange(p, desired);
}

inline static
void* ax_atomic_ptr_load(ax_atm_void_ptr* p)
{
    return _InterlockedCompareExchangePointer(p, 0, 0);
}

inline static
void ax_atomic_ptr_store(ax_atm_void_ptr* p, void* desired)
{
    _InterlockedExchangePointer(p, desired);
}

inline static
void* ax_atomic_ptr_xchg(ax_atm_void_ptr* p, void* desired)
{
    return _InterlockedExchangePointer(p, desired);
}

#endif

#endif/*_AX_ATOMIC_H_*/
