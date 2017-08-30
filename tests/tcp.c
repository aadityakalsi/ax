/*
For license details see ../../LICENSE
*/

/**
 * \file tcp.c
 * \date Aug 15, 2017
 */

#include "defs.h"

#include "ax/assert.h"
#include "ax/tcp.h"
#include "ax/thread.h"
#include "ax/atomic.h"

#define AX_MIN_LOG_LEVEL AX_LOG_DBUG
#include "ax/log.h"

#include <string.h>

/*--------------------------------------------------*/

void create_destroy_srv(void)
{
    ax_tcp_srv_t srv;
    testThat(ax_tcp_srv_init_ip4(&srv, "localhost", 8000) == 0);
    testThat(ax_tcp_srv_destroy(&srv) == 0);
}

/*--------------------------------------------------*/

void create_destroy_cli(void)
{
    ax_tcp_cli_t cli;
    testThat(ax_tcp_cli_init_ip4(&cli, "localhost", 8001) == 0);
    testThat(ax_tcp_cli_destroy(&cli) == 0);
}

/*--------------------------------------------------*/

typedef struct
{
    int client_wrote;
    int server_wrote;
    int client_write_status;
    int server_write_status;
    int client_read_status;
} srv_cli_status_t;

static ax_atomic_i32 start_connect = 0;
static srv_cli_status_t test;
static ax_tcp_srv_t srv;
static ax_tcp_cli_t cli;

static
void wait_and_connect(void* a)
{
    ax_tcp_cli_t* c = &cli;
    ax_thread_t _a, _b;
    while (ax_atomic_i32_load(&start_connect) != 1) { }
    _a = ax_thread_self();
    _b = ax_thread_self();
    AX_INVARIANT(ax_thread_equal(&_a, &_b));
    ax_tcp_cli_connect(c);
}

static
void server_ctx_get_buf(void* c, ax_buf_t* buf)
{
    static char _buf[48];
    int* state = (int*)c;
    switch (*state) {
        case 0:
            buf->data = _buf;
            buf->len = 48;
            break;
        case 1:
            buf->data = AX_STOP_READ_START_WRITE;
            break;
        case 2:
            buf->data = "Hello, client!\n";
            buf->len = (ax_i32)strlen(buf->data);
            break;
        case 3:
            buf->data = AX_STOP_WRITE_START_READ;
            break;
        case 4:
            buf->data = _buf;
            buf->len = 48;
            break;
        default:
            AX_ASSERT(*state == 5);
            ax_tcp_srv_stop(&srv);
            break;
    }
    ++(*state);
}

static
void server_ctx_free_buf(void* c, ax_buf_t const* buf)
{
}

static
void server_read_cbk(void* c, int err, ax_buf_t const* b)
{
    int* state = (int*)c;
    if (*state == 1) {
        AX_INVARIANT(strncmp(b->data,"Hello, server!\n", b->len) == 0);
    } else {
        AX_INVARIANT(*state == 5);
        AX_INVARIANT(strncmp(b->data, "Bye!\n", b->len) == 0);
    }
}

static
void server_write_cbk(void* c, int err)
{
    test.server_wrote = 1;
    test.server_write_status = err;
}

static
void server_on_start(void* u)
{
    ax_atomic_i32_store(&start_connect, 1);
}

static
void server_init_req(void* u, ax_tcp_req_t* req)
{
    int* p = (int*)malloc(sizeof(int));
    *p = 0;
    req->req_ctx = p;
    req->get_read_buf = server_ctx_get_buf;
    req->free_read_buf = server_ctx_free_buf;
    req->get_write_buf = server_ctx_get_buf;
    req->free_write_buf = server_ctx_free_buf;
    req->read_cbk = server_read_cbk;
    req->write_cbk = server_write_cbk;
}

static
void server_destroy_req(void* u, ax_tcp_req_t* req)
{
    int* p = (int*)req->req_ctx;
    AX_ASSERT(*p == 6);
    free(req->req_ctx);
}

static
void client_ctx_get_buf(void* c, ax_buf_t* buf)
{
    static char _buf[48];
    int* state = (int*)c;
    switch (*state) {
        case 0:
            buf->data = "Hello, server!\n";
            buf->len = (ax_i32)strlen(buf->data);
            break;
        case 1:
            buf->data = AX_STOP_WRITE_START_READ;
            break;
        case 2:
            buf->data = _buf;
            buf->len = 48;
            break;
        case 3:
            buf->data = AX_STOP_READ_START_WRITE;
            break;
        case 4:
            buf->data = "Bye!\n";
            buf->len = (ax_i32)strlen(buf->data);
            break;
        default:
            AX_ASSERT(*state == 5);
            buf->data = AX_STOP_WRITE_NO_READ;
            break;
    }
    ++(*state);
}

static
void client_ctx_free_buf(void* c, ax_buf_t const* buf)
{
}

static
void client_read_cbk(void* c, int err, ax_buf_t const* b)
{
    test.client_read_status = err;
}

static
void client_write_cbk(void* c, int err)
{
    test.client_wrote = 1;
    test.client_write_status = err;
}

static
void client_on_start(void* u)
{
}

static
void client_init_req(void* u, ax_tcp_req_t* req)
{
    int* p = (int*)malloc(sizeof(int));
    *p = 0;
    req->req_ctx = p;
    req->get_read_buf = client_ctx_get_buf;
    req->free_read_buf = client_ctx_free_buf;
    req->get_write_buf = client_ctx_get_buf;
    req->free_write_buf = client_ctx_free_buf;
    req->read_cbk = client_read_cbk;
    req->write_cbk = client_write_cbk;
}

static
void client_destroy_req(void* u, ax_tcp_req_t* req)
{
    int* p = (int*)req->req_ctx;
    AX_ASSERT(*p == 6);
    free(req->req_ctx);
}

void create_connect_destroy(void)
{
    ax_thread_t cli_tid;
    ax_tcp_ctx_t srv_ctx = {
        AX_NULL,
        server_on_start,
        server_init_req,
        server_destroy_req
    };
    ax_tcp_ctx_t cli_ctx = {
        AX_NULL,
        client_on_start,
        client_init_req,
        client_destroy_req
    };

    test.client_wrote = 0;
    test.server_wrote = 0;
    test.client_write_status = -1;
    test.server_write_status = -1;
    test.client_read_status = -1;
    ax_atomic_i32_store(&start_connect, 0);

    testThat(ax_tcp_srv_init_ip4(&srv, "localhost", 8080) == 0);
    ax_tcp_srv_set_ctx(&srv, &srv_ctx);
    testThat(ax_tcp_cli_init_ip4(&cli, "localhost", 8080) == 0);
    ax_tcp_cli_set_ctx(&cli, &cli_ctx);
    testThat(ax_thread_create(&cli_tid, wait_and_connect, AX_NULL) == 0);
    testThat(ax_tcp_srv_start(&srv) == 1);
    testThat(ax_thread_join(&cli_tid) == 0);
    ax_tcp_cli_stop(&cli);
    testThat(ax_tcp_cli_destroy(&cli) == 0);
    testThat(ax_tcp_srv_destroy(&srv) == 0);
    testThat(test.client_wrote == 1);
    testThat(test.server_wrote == 1);
    testThat(test.client_write_status == 0);
    testThat(test.server_write_status == 0);
    testThat(test.client_read_status == 0);
}

/*--------------------------------------------------*/

/*--------------------------------------------------*/
setupSuite(tcp)
{
    addTest(create_destroy_srv);
    addTest(create_destroy_cli);
    addTest(create_connect_destroy);
}
