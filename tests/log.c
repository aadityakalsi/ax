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

void log_stderr(void)
{
    fprintf(stderr, "\n");
    AX_LOG(INFO, "abcdefghi\n");
    AX_LOG(DBUG, "foo!! %d\n", 129);
    AX_LOG(WARN, "jklmnopqrstuvwx\n");
    AX_LOG(FAIL, "a failure: %d\n", -1);
    testThat(1);
}

setupSuite(log)
{
    addTest(log_stderr);
}
