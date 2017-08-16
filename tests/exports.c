/*! exports.c */

#include "defs.h"

#include "ax/types.h"

#ifdef AX_C_API
#  define VER_DEF_FOUND 1
#else
#  define VER_DEF_FOUND 0
#endif

void exports_h(void)
{
    testThat(1 == VER_DEF_FOUND);
}

void types_h(void)
{
    testThat(0 == AX_NULL);
}

setupSuite(exports)
{
    addTest(exports_h);
    addTest(types_h);
}
