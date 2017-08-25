/*
For license details see ../../LICENSE
*/

/**
 * \file log.c
 * \date Aug 11, 2017
 */

#include "defs.h"

#define AX_MIN_LOG_LEVEL AX_LOG_INFO
#include "ax/log.h"

#include <string.h>

static
void log_stderr(void)
{
    fprintf(stderr, "\n");
    AX_LOG(INFO, "abcdefghi\n");
    AX_LOG(DBUG, "foo!! %d\n", 129);
    AX_LOG(WARN, "jklmnopqrstuvwx\n");
    AX_LOG(FAIL, "a failure: %d\n", -1);
    testThat(1);
}

static
void log_file(void)
{
    ax_const_str fname = "foo.log";
    FILE* f = fopen(fname, "wb");
    char line[80];
    size_t nread;
    if (!f) return;
    ax_set_log_file(f);
    AX_LOG(INFO, "MAGIC\n");
    ax_set_log_file(stderr);
    fclose(f);
    f = fopen(fname, "rb");
    nread = fread(line, 1, 80, f);
    fclose(f);
    testThat(remove(fname) == 0);
    testThat(strcmp(&line[nread-6], "MAGIC\n") == 0);
}

setupSuite(log)
{
    addTest(log_stderr);
    addTest(log_file);
}
