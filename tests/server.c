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

void srvr_create_destroy(void)
{
    ax_server_t srv;
    testThat(ax_server_init_ip4(&srv, "localhost", 8080) == 0);
    testThat(ax_server_destroy(&srv) == 0);
}

/*--------------------------------------------------*/

typedef struct
{
    ax_client_t client;
    ax_u32 conn;
    ax_u32 conn_ack;
} wait_client_t;

static
void wait_and_connect(void* a)
{
    wait_client_t* c = (wait_client_t*)a;
    while (ax_aload_u32(&c->conn) != 1) { }
    ax_astore_u32(&c->conn_ack, 1);
    ax_client_run_once(&c->client);
}

void srvr_create_connect_destroy(void)
{
    ax_server_t srv;
    wait_client_t cli;
    ax_thread_t cli_tid;
    cli.conn = 0;
    cli.conn_ack = 0;
    testThat(ax_server_init_ip4(&srv, "localhost", 8080) == 0);
    testThat(ax_client_init_ip4(&cli.client, "localhost", 8080) == 0);
    testThat(ax_thread_create(&cli_tid, wait_and_connect, &cli) == 0);
    ax_astore_u32(&cli.conn, 1);
    while (ax_aload_u32(&cli.conn_ack) == 0) { }
    testThat(ax_server_run_once(&srv) == 1);
    testThat(ax_server_destroy(&srv) == 0);
    testThat(ax_thread_join(&cli_tid) == 0);
    testThat(ax_client_destroy(&cli.client) == 0);
}

/*--------------------------------------------------*/
setupSuite(server)
{
    addTest(srvr_create_destroy);
    addTest(srvr_create_connect_destroy);
}
