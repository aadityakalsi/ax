/*! defs.h */

#if defined(_MSC_VER)
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>

#ifdef __cplusplus
#  define EXTERN_C extern "C"
#  define setupSuite(name) EXTERN_C void name##SuiteSetup()
#else
#  define EXTERN_C
#  define setupSuite(name) void name##SuiteSetup()
#endif

typedef void (*test_fn_t)(void);

struct test_s;
typedef struct test_s test_t;

struct test_s
{
    char const* suite;
    char const* name;
    test_fn_t fn;
};

#ifdef _WIN32
EXTERN_C
#endif
extern jmp_buf JMP_BUF;

#ifdef _WIN32
EXTERN_C
#endif
extern char* ERR_BUF;

EXTERN_C
void _add_test(char const* name, test_fn_t fn);

#define testThat(expr)        \
    if (!(expr)) {            \
        sprintf(ERR_BUF, "        FAIL: `%s' at %s:%d\n", #expr, __FILE__, __LINE__); \
        longjmp(JMP_BUF, 1);  \
    }

#define addTest(name)         \
    _add_test(#name, name)

#define defineSuite(name)     \
    EXTERN_C void name##SuiteSetup()
