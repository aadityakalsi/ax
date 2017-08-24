/*
For license details see ../../LICENSE
*/

/**
 * \file log.h
 * \date Aug 11, 2017
 */

#ifndef _AX_LOG_H_
#define _AX_LOG_H_

#include "ax/types.h"

#include <stdio.h>

#define AX_LOG_INFO  0
#define AX_LOG_DBUG  1
#define AX_LOG_WARN  2
#define AX_LOG_FAIL  3
#define AX_LOG_NONE  100

#ifndef AX_MIN_LOG_LEVEL
#  ifndef NDEBUG
#    define AX_MIN_LOG_LEVEL AX_LOG_DBG
#  else
#    define AX_MIN_LOG_LEVEL AX_LOG_WARN
#  endif
#endif

#define __AX_STR_X(x) #x
#define __AX_STR(x) __AX_STR_X(x)

#define AX_LOG(lvl, fmt, ...)                                                   \
  if (AX_LOG_##lvl >= AX_MIN_LOG_LEVEL) {                                       \
      char __xx_buf[] = "["#lvl"] (" __FILE__ ":" __AX_STR(__LINE__) ") " fmt;  \
      _ax_print_log(__xx_buf, ## __VA_ARGS__);                                  \
  }                                                                             \
  do { } while (0)

AX_API
void ax_set_log_file(FILE* f);

AX_API
FILE* ax_get_log_file();

AX_API
void _ax_print_log(ax_str fmt, ...);

#endif/*_AX_LOG_H_*/
