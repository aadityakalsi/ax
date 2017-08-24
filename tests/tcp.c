/*
For license details see ../../LICENSE
*/

/**
 * \file tcp.c
 * \date Aug 15, 2017
 */

#include "defs.h"

#include "ax/tcp.h"
#include "ax/thread.h"
#include "ax/atomic.h"

#define AX_MIN_LOG_LEVEL AX_LOG_DBUG
#include "ax/log.h"

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
    int status_server;
    int status_client;
} srv_cli_status_t;

static ax_atomic_i32 start_connect = 0;
static srv_cli_status_t test;
static ax_tcp_srv_t srv;
static ax_tcp_cli_t cli;

static
void client_on_connect(void* u, int status)
{
    ((srv_cli_status_t*)u)->status_client = status;
    ax_tcp_cli_stop(&cli);
}

static
void server_on_listen(void* u)
{
    ax_atomic_i32_store(&start_connect, 1);
    AX_LOG(DBUG, "listen done!\n");
}

static
void server_on_connect(void* u, int status)
{
    ((srv_cli_status_t*)u)->status_server = status;
    ax_tcp_srv_stop(&srv);
}

static
void wait_and_connect(void* a)
{
    ax_tcp_cli_t* c = &cli;
    while (ax_atomic_i32_load(&start_connect) != 1) { }
    ax_tcp_cli_connect(c);
}

void create_connect_destroy(void)
{
    ax_thread_t cli_tid;
    ax_tcp_srv_cbk_t srv_cbk = {
        &test,
        server_on_listen,
        server_on_connect
    };
    ax_tcp_cli_cbk_t cli_cbk = {
        &test,
        client_on_connect
    };

    test.status_server = -1;
    test.status_client = -1;
    ax_atomic_i32_store(&start_connect, 0);

    testThat(ax_tcp_srv_init_ip4(&srv, "localhost", 8080) == 0);
    ax_tcp_srv_set_cbk(&srv, &srv_cbk);
    testThat(ax_tcp_cli_init_ip4(&cli, "localhost", 8080) == 0);
    ax_tcp_cli_set_cbk(&cli, &cli_cbk);
    testThat(ax_thread_create(&cli_tid, wait_and_connect, AX_NULL) == 0);
    testThat(ax_tcp_srv_start(&srv) == 1);
    testThat(ax_thread_join(&cli_tid) == 0);
    testThat(ax_tcp_cli_destroy(&cli) == 0);
    testThat(ax_tcp_srv_destroy(&srv) == 0);
    testThat(test.status_server == 0);
    testThat(test.status_client == 0);
}

/*--------------------------------------------------*/

/*--------------------------------------------------*/
setupSuite(tcp)
{
    addTest(create_destroy_srv);
    addTest(create_destroy_cli);
    addTest(create_connect_destroy);
}
