/*
For license details see ../LICENSE
*/

/**
 * \file tcp_server.c
 * \date Aug 24, 2017
 */

#define AX_MIN_LOG_LEVEL AX_LOG_FAIL
#include "ax/tcp.h"
#include "ax/log.h"
#include "ax/assert.h"

#include <stdlib.h>
#include <string.h>

#define CHECK(x)   \
  {                \
    int ret = (x); \
    if (ret) {     \
        AX_LOG(FAIL, #x ": %s\n", ax_error_str(ret)); \
        return ret;\
    }              \
  }

typedef struct
{
    int state;
} response_t;

static ax_tcp_srv_t server;
static int replied = 0;

static
void read_data(void* u, int status, ax_buf_t* buf)
{
    buf->data = AX_STOP_READ_START_WRITE;
    replied = 0;
}

static
void write_data(void* u, ax_buf_t* buf)
{
    if (!replied) {
        buf->data = "hello, client!\n";
        buf->len = (ax_i32) strlen(buf->data);
        replied = 1;
    } else {
        buf->data = AX_STOP_WRITE_NO_READ;
    }
}

#if defined(AX_BUILD_COVERAGE)
extern void __gcov_flush (void);

#include <signal.h>

void my_handler(int signum)
{
    printf("received signal: %d\n", signum);
    if (signum == SIGUSR1) {
        __gcov_flush(); /* dump coverage data on receiving SIGUSR1 */
    }
}

static
void signal_setup(void)
{
    struct sigaction new_action, old_action;
    new_action.sa_handler = my_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGUSR1, NULL, &old_action);
    //if (old_action.sa_handler != SIG_IGN) {
        sigaction(SIGUSR1, &new_action, NULL);
    //}
}
#else
void signal_setup(void)
{
}
#endif


int main(int argc, ax_const_str argv[])
{
    ax_const_str ip   = argc > 1 ? argv[1] : "localhost";
    ax_const_str port = argc > 2 ? argv[2] : "8080";
    ax_tcp_srv_cbk_t cbk = {
            /*void* userdata;*/ AX_NULL,
            /*void (*listen_fn)(void* userdata);*/ AX_NULL,
            /*void (*connect_fn)(void* userdata, int status);*/ AX_NULL,
            /*void (*accept_fn)(void* userdata, ax_sz clientid);*/ AX_NULL,
            /*void (*data_fn)(void* userdata, int status, ax_buf_t* buf);*/ read_data,
            /*void (*write_data_fn)(void* userdata, ax_buf_t* buf);*/ write_data,
            /*void (*write_fn)(void* userdata, int status);*/ AX_NULL
    };
    ax_set_log_file(fopen("/tmp/tcp_server.log", "wb"));
    signal_setup();
    CHECK(ax_tcp_srv_init_ip4(&server, ip, atoi(port)));
    ax_tcp_srv_set_cbk(&server, &cbk);
    ax_tcp_srv_start(&server);
    CHECK(ax_tcp_srv_destroy(&server));
    return 0;
}
