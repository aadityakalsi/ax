/*
For license details see ../../LICENSE
*/

/**
 * \file log.c
 * \date Aug 11, 2017
 */

#include "ax/assert.h"
#include "ax/log.h"
#include "ax/atomic.h"

#include <string.h>
#include <stdarg.h>

static FILE* g_log_file = (FILE*)-1;
static ax_atomic_i32 g_lock = 0;
static char __cache_align[64];
static char g_buff[8192];

void ax_set_log_file(FILE* f)
{
    if (g_log_file != f) {
        g_log_file = f;
        if (f != stderr && f != (FILE*)-1 && f != 0) {
            setvbuf(f, g_buff, _IOLBF, sizeof(g_buff));
        }
    }
}

FILE* ax_get_log_file(void)
{
    return (g_log_file == (FILE*)-1) ? stderr : g_log_file;
}

void _ax_print_log(ax_str fmt, ...)
{
    FILE* const f = ax_get_log_file();
    ax_str p = fmt;
    ax_str last;
    ax_str e;
    va_list args;

    if (!f) return;

    p = strchr(fmt, '(');
    e = strchr(p+1, ')');
    *e = '\0';
    last = strrchr(p+1, '/');
#ifdef _WIN32
    last = last ? last : strrchr(p+1,'\\');
#endif
    *e = ')';
    AX_ASSERT(last);
    strcpy(p+1, last+1);

    while (ax_atomic_i32_xchg(&g_lock, 1) == 1) { }
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    ax_atomic_i32_store(&g_lock, 0);
}
