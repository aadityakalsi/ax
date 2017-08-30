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
            setvbuf(f, g_buff, _IOFBF, sizeof(g_buff));
        }
    }
}

FILE* ax_get_log_file(void)
{
    return (g_log_file == (FILE*)-1) ? stderr : g_log_file;
}

static
void* _ax_memrchr(void const* b, int c, size_t l)
{
    register ax_const_str const s = (ax_const_str)b;
    register ax_const_str end = s + l;
    for (; end > s; --end) {
        if (*(end-1) == c) {
            return end-1;
        }
    }
    return AX_NULL;
}

void _ax_print_log(ax_sz str_len, ax_str fmt, ...)
{
    FILE* const f = ax_get_log_file();
    ax_str p = fmt;
    ax_str last;
    ax_str e;
    va_list args;

    if (!f) return;

    p = (ax_str)memchr(fmt, '(', str_len);
    e = (ax_str)memchr(p+1, ')', str_len-(p+1-fmt));
    AX_ASSERT(p);
    AX_ASSERT(e);
    last = (ax_str)_ax_memrchr(p+1, '/', e-(p+1));
#ifdef _WIN32
    last = last ? last : (ax_str)_ax_memrchr(p+1,'\\',e-(p+1));
#endif
    if (last) {
        AX_ASSERT(last);
        memmove(p+1, last+1, str_len-(p+2-fmt));
    }

    while (ax_atomic_i32_xchg(&g_lock, 1) == 1) { }
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    ax_atomic_i32_store(&g_lock, 0);
}
