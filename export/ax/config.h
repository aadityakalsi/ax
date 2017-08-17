/*
For license details see ../../LICENSE
*/

/**
 * \file config.h
 * \date Aug 16, 2017
 */

#ifndef _AX_CONFIG_H_
#define _AX_CONFIG_H_

/*
 * Compiler definition macros
 */
#if defined(_MSC_VER)
#  define AX_MSVC       1
#endif

#if defined(__GNUC__) && !defined(__clang__)
#  define AX_GCC        1
#endif

#if defined(__clang__)
#  define AX_CLANG      1
#endif

#if !AX_MSVC && !AX_GCC && !AX_CLANG
#  error "Unsupported compiler!"
#endif

/*
 * Platform definition macros
 */
#if defined(_WIN32)
#  define AX_LINUX      0
#  define AX_WINDOWS    1
#  define AX_IOS        0
#  define AX_MACOS      0
#endif

#if defined(__linux__)
#  define AX_LINUX      1
#  define AX_WINDOWS    0
#  define AX_IOS        0
#  define AX_MACOS      0
#endif

#if defined(__APPLE__)
#include <TargetConditionals.h>
#  if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
#    define AX_LINUX    0
#    define AX_WINDOWS  0
#    define AX_IOS      1
#    define AX_MACOS    0
#  elif TARGET_OS_MAC
#    define AX_LINUX    0
#    define AX_WINDOWS  0
#    define AX_IOS      0
#    define AX_MACOS    1
#  else
#    error "Unknown Apple platform"
#  endif
#endif

#if !AX_LINUX && !AX_WINDOWS && !AX_IOS && !AX_MACOS
#  error "Invalid or unsupported platform!"
#endif

/*
 * Bit-ness definition macros
 * see: https://sourceforge.net/p/predef/wiki/Architectures/
 */
#if defined(_WIN32)
#  if defined(_WIN64)
#    define AX_BITNESS 64
#  else
#    define AX_BITNESS 32
#  endif
#endif

#if AX_GCC || AX_CLANG
#  if defined(__LP64__) || defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__) || defined(__ARM_ARCH_ISA_A64)
#    define AX_BITNESS 64
#  else
#    define AX_BITNESS 32
#  endif
#endif

#if !defined(AX_BITNESS) || (AX_BITNESS != 32 && AX_BITNESS != 64)
#  error "Invalid bitness or bitness undefined."
#endif

/*
 * ISA definition macros
 * see: https://sourceforge.net/p/predef/wiki/Architectures/
 */
#if defined(__arm__) || defined(_M_ARM)
#  define AX_ARM        1
#else
#  define AX_ARM        0
#endif

#endif/*_AX_CONFIG_H_*/
