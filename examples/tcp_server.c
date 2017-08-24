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

#define CHECK(x)   \
  {                \
    int ret = (x); \
    if (ret) {     \
        AX_LOG(FAIL, #x ": %s\n", ax_error_str(ret)); \
        return ret;\
    }              \
  }

static ax_tcp_srv_t server;

int main(int argc, ax_const_str argv[])
{
    ax_const_str ip   = argc > 1 ? argv[1] : "localhost";
    ax_const_str port = argc > 2 ? argv[2] : "8080";
    ax_tcp_srv_cbk_t cbk = {
        AX_NULL,
        AX_NULL,
        AX_NULL,
        AX_NULL
    };
    CHECK(ax_tcp_srv_init_ip4(&server, ip, atoi(port)));
    ax_tcp_srv_set_cbk(&server, &cbk);
    ax_tcp_srv_start(&server);
    CHECK(ax_tcp_srv_destroy(&server));
    return 0;
}
