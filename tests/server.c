/*
For license details see ../../LICENSE
*/

/**
 * \file server.c
 * \date Aug 15, 2017
 */

#include "defs.h"

#include "ax/server.h"
#include "ax/client.h"
#include "ax/thread.h"
#include "ax/atomic.h"

#define AX_MIN_LOG_LEVEL AX_LOG_DBUG
#include "ax/log.h"

/*--------------------------------------------------*/

void create_destroy(void)
{
    ax_server_t srv;
    testThat(ax_server_init_ip4(&srv, "localhost", 8000) == 0);
    testThat(ax_server_destroy(&srv) == 0);
}

/*--------------------------------------------------*/

typedef struct
{
    int status_server;
    int status_client;
} srv_cli_status_t;

static ax_atm_i32 start_connect = 0;
static srv_cli_status_t test;

static
void client_on_connect(void* u, int status)
{
    ((srv_cli_status_t*)u)->status_client = status;
    ax_atomic_i32_store(&start_connect, 0);
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
}

static
void wait_and_connect(void* a)
{
    ax_client_t* c = (ax_client_t*)a;
    while (ax_atomic_i32_load(&start_connect) != 1) { }
    ax_client_run_once(c);
    ax_client_run_once(c);
}

void create_connect_destroy(void)
{
    ax_server_t srv;
    ax_client_t cli;
    ax_thread_t cli_tid;
    ax_server_cbk_t srv_cbk = {
        &test,
        server_on_listen,
        server_on_connect
    };
    ax_client_cbk_t cli_cbk = {
        &test,
        client_on_connect
    };

    test.status_server = 500;
    test.status_client = 500;
    ax_atomic_i32_store(&start_connect, 0);

    ax_server_set_callbacks(&srv, &srv_cbk);
    ax_client_set_callbacks(&cli, &cli_cbk);
    testThat(ax_server_init_ip4(&srv, "localhost", 8080) == 0);
    testThat(ax_client_init_ip4(&cli, "localhost", 8080) == 0);
    testThat(ax_thread_create(&cli_tid, wait_and_connect, &cli) == 0);
    testThat(ax_server_run_once(&srv) == 1);
    while (ax_atomic_i32_load(&start_connect) != 0) { }
    testThat(ax_thread_join(&cli_tid) == 0);
    testThat(ax_client_destroy(&cli) == 0);
    testThat(ax_server_destroy(&srv) == 0);
    testThat(test.status_server != 500);
    testThat(test.status_client != 500);
}

/*--------------------------------------------------*/

/*--------------------------------------------------*/
setupSuite(server)
{
    addTest(create_destroy);
    addTest(create_connect_destroy);
}
