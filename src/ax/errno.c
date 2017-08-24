/*
For license details see ../../LICENSE
*/

/**
 * \file errno.c
 * \date Aug 23, 2017
 */

#include "ax/errno.h"

#include <uv.h>

ax_const_str ax_error_str(int err)
{
    return uv_strerror(err);
}

ax_const_str ax_error_name(int err)
{
    return uv_err_name(err);
}
