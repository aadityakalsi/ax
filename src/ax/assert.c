/*
For license details see ../../LICENSE
*/

/**
 * \file assert.c
 * \date Aug 11, 2017
 */

#include "ax/assert.h"

#include <stdio.h>
#include <stdlib.h>

/*LCOV_EXCL_START*/

void _ax_assert_failed(ax_const_str hdr, ax_const_str msg)
{
    fprintf(stderr, "\nASSERT FAIL: %s%s %s\n", hdr, msg ? ";" : "", msg ? msg : "");
    abort();
}

/*LCOV_EXCL_STOP*/
