/*! driver.c */

#include "defs.h"

static test_t TESTS[4096];
static char ERRBUF[4096];
static int TOTAL = 0;

#define runSuite(name)        \
    void name##SuiteSetup();  \
    SUITE = #name;            \
    name##SuiteSetup();       \

static char const* SUITE = 0;

jmp_buf JMP_BUF;
char* ERR_BUF = &ERRBUF[0];

void _add_test(char const* name, test_fn_t fn)
{
    TESTS[TOTAL].suite = SUITE;
    TESTS[TOTAL].name = name;
    TESTS[TOTAL].fn = fn;
    ++TOTAL;
}

static int nfail = 0;
static char const* suite = 0;
static int i = 0;

int main(int argc, char const* argv[])
{
    (void)argc; (void)argv;

    #include "suites.h"

    for (i = 0; i < TOTAL; ++i) {
        if (suite != TESTS[i].suite) {
            if (suite) {
                printf("endsuite: %s\n\n", suite);
            } else {
                printf("\n");
            }
            suite = TESTS[i].suite;
            printf("suite: %s\n", suite);
        }
        printf(" %3d  %s: ", i, TESTS[i].name);
        fflush(stdout);
        int val = setjmp(JMP_BUF);
        if (val == 0) {
            TESTS[i].fn();
            printf("OK\n");
            continue;
        } else {
            printf("FAIL\n%s", ERR_BUF);
            nfail++;
        }
    }

    if (suite) {
        printf("endsuite: %s\n", suite);
    }

    if (nfail == 0) {
        printf("\nTESTS PASSED: %d\n", TOTAL);
        printf("ALL OK\n");
    } else {
        printf("\nTESTS FAILED: %d\n", nfail);
        printf("FAILED\n");
    }
    printf("\n");
    return nfail;
}
