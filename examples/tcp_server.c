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
void req_get_buf(void* c, ax_buf_t* buf)
{
    int* p = (int*)c;
    static char _buf[4096];
    if (*p == 0) {
        buf->data = _buf;
        buf->len = 4096;
        ++(*p);
    } else if (*p == 1) {
        ++(*p);
        buf->data = AX_STOP_READ_START_WRITE;
    } else if (*p == 2) {
        ++(*p);
        buf->data = "Goodbye client!\n";
        buf->len = strlen(buf->data);
    } else {
        AX_ASSERT(*p == 3);
        buf->data = AX_STOP_WRITE_NO_READ;
    }
}

static
void req_free_buf(void* c, ax_buf_t const* buf)
{
}

static
void req_read_cbk(void* u, int err, ax_buf_t const* b)
{
}

static
void req_write_cbk(void* u, int err)
{
}

static
void srv_on_start(void* u)
{
    AX_LOG(DBUG, "Server started!\n");
}

static
void srv_init_req(void* u, ax_tcp_req_t* req)
{
    int* p = malloc(sizeof(int));
    *p = 0;
    ax_tcp_req_t r = {
        p,
        req_get_buf,
        req_free_buf,
        req_get_buf,
        req_free_buf,
        req_read_cbk,
        req_write_cbk
    };
    *req = r;
}

static
void srv_destroy_req(void* u, ax_tcp_req_t* req)
{
    free(req->req_ctx);
}

#if defined(AX_BUILD_COVERAGE)
extern void __gcov_flush(void);

#include <signal.h>

void sig_hdlr(int signum)
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
    new_action.sa_handler = sig_hdlr;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGUSR1, NULL, &old_action);
    sigaction(SIGUSR1, &new_action, NULL);
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
    ax_tcp_srv_ctx_t ctx = {
        AX_NULL,
        srv_on_start,
        srv_init_req,
        srv_destroy_req
    };
    ax_set_log_file(fopen("/tmp/tcp_server.log", "wb"));
    signal_setup();
    CHECK(ax_tcp_srv_init_ip4(&server, ip, atoi(port)));
    ax_tcp_srv_set_ctx(&server, &ctx);
    ax_tcp_srv_start(&server);
    CHECK(ax_tcp_srv_destroy(&server));
    return 0;
}
