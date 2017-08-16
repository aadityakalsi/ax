/*
For license details see ../../LICENSE
*/

/**
 * \file log.c
 * \date Aug 11, 2017
 */

#include "ax/assert.h"
#include "ax/log.h"

#include <stdarg.h>

static FILE* g_log_file = (FILE*)-1;

void ax_set_log_file(FILE* f)
{
    g_log_file = f;
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

    while (*p++ != '(') {
        AX_ASSERT(*p != '\0');
    }

    // start past the '('
    e = p;
    last = p;

    while (*e != ')') {
        AX_ASSERT(*e != '\0');
        if (*e == '/'
#ifdef _WIN32
         || *e == '\\'
#endif
           ) {
            last = e;
        }
        ++e;
    }

    if (last != p) {
        // found a '/' before ')'
        ++last; // move past filesep
        while (*last != '\0') { *p++ = *last++; }
        *p = '\0';
    }

    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
}
