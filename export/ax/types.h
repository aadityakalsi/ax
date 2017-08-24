/*
For license details see ../../LICENSE
*/

/**
 * \file types.h
 * \date Aug 11, 2017
 */

#ifndef _AX_TYPES_H_
#define _AX_TYPES_H_

#include "ax/config.h"
#include "ax/exports.h"
#include "ax/errno.h"

#include <stddef.h>
#include <stdint.h>

typedef uint8_t     ax_u8 ;
typedef int8_t      ax_i8 ;
typedef uint16_t    ax_u16;
typedef int16_t     ax_i16;
typedef uint32_t    ax_u32;
typedef int32_t     ax_i32;
typedef uint64_t    ax_u64;
typedef int64_t     ax_i64;
typedef size_t      ax_sz ;
typedef double      ax_dbl;
typedef float       ax_flt;

#define AX_NULL (0)

#define AX_STRUCT_TYPE(ty)     \
  typedef struct _##ty ty;     \
  struct _##ty

#define AX_HIDDEN_TYPE(ty, sz) \
  AX_STRUCT_TYPE(ty)           \
  {                            \
      union {                  \
      char __data[sz];         \
      double __align;          \
      };                       \
  }

#endif/*_AX_TYPES_H_*/
