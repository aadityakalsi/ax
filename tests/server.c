/*
For license details see ../../LICENSE
*/

/**
 * \file server.c
 * \date Aug 15, 2017
 */

#include "defs.h"

#include "ax/server.h"

void srvr_create_destroy(void)
{
    ax_server_t srv;
    testThat(ax_server_init_ip4(&srv, "localhost", 8080) == 0);
    testThat(ax_server_destroy(&srv) == 0);
}

void srvr_create_connect_destroy(void)
{
    ax_server_t srv;
    testThat(ax_server_init_ip4(&srv, "localhost", 8080) == 0);
    testThat(ax_server_run_once(&srv) == 1);
    testThat(ax_server_destroy(&srv) == 0);
}

setupSuite(server)
{
    addTest(srvr_create_destroy);
    addTest(srvr_create_connect_destroy);
}
