/*
For license details see ../../LICENSE
*/

/**
 * \file tcp-common.c
 * \date Aug 25, 2017
 */

#include "ax/tcp-common.h"
#include "ax/assert.h"
#include "ax/log.h"
#include "ax/atomic.h"
#include "ax/pool.h"

#include <string.h>

AX_STATIC_ASSERT(sizeof(sockaddr_t) >= sizeof(struct sockaddr_in6), sockaddr_too_small);

int _init_loop_and_tcp(uv_loop_t* loop, uv_tcp_t* tcp, sockaddr_t* saddr, ax_const_str addr, int port)
{
    int ret;
    struct addrinfo hints;
    uv_getaddrinfo_t addrinfo;
    char port_str[12];

    if ((ret = uv_loop_init(loop))) {
        AX_LOG(DBUG, "loop: %s\n", ax_error_str(ret));
        goto _init_loop_and_tcp_done;
    }

    if ((ret = uv_tcp_init(loop, tcp))) {
        AX_LOG(DBUG, "tcp_init: %s\n", ax_error_str(ret));
        goto _init_loop_and_tcp_done;
    }

    snprintf(port_str, 12, "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = 0;
    hints.ai_flags = AI_PASSIVE;
    if ((ret = uv_getaddrinfo(loop, &addrinfo, AX_NULL, addr, port_str, &hints))) {
        AX_LOG(DBUG, "getaddrinfo: %s\n", ax_error_str(ret));
        goto _init_loop_and_tcp_done;
    }
    memcpy(saddr, addrinfo.addrinfo->ai_addr, addrinfo.addrinfo->ai_addrlen);
    uv_freeaddrinfo(addrinfo.addrinfo);

_init_loop_and_tcp_done:
    return ret;
}

void _close_all_handles(uv_handle_t* h, void* unused)
{
    AX_ASSERT(unused == AX_NULL);
    if (!uv_is_closing(h) && uv_is_active(h)) {
        uv_close(h, h->close_cb);
    }
}

int _close_loop(uv_loop_t* loop)
{
    int ret = 0;
    if (!uv_loop_alive(loop)) {
        goto _close_loop_done;
    }
    uv_walk(loop, _close_all_handles, AX_NULL);
    if ((ret = uv_run(loop, UV_RUN_DEFAULT))) {
        AX_LOG(DBUG, "run_default: %s\n", ax_error_str(ret));
        goto _close_loop_done;
    }
    if ((ret = uv_loop_close(loop))) {
        AX_LOG(DBUG, "loop_close: %s\n", ax_error_str(ret));
        goto _close_loop_done;
    }

_close_loop_done:
    return ret;
}

static ax_atomic_i32 CLIENT_POOL_LOCK = 0;
static ax_pool_t CLI_CONN_POOL_STORAGE;
static ax_pool_t* CLI_CONN_POOL = AX_NULL;

static
ax_pool_t* _get_client_conn_pool(void)
{
    if (!CLI_CONN_POOL) {
        while (ax_atomic_i32_xchg(&CLIENT_POOL_LOCK, 1) == 1) { }
        if (!CLI_CONN_POOL && ax_pool_init(&CLI_CONN_POOL_STORAGE, sizeof(tcp_cli_conn_t)) == 0) {
            CLI_CONN_POOL = &CLI_CONN_POOL_STORAGE;
        }
        ax_atomic_i32_store(&CLIENT_POOL_LOCK, 0);
    }
    return CLI_CONN_POOL;
}

tcp_cli_conn_t* _create_tcp_client(void)
{
    ax_pool_t* p = _get_client_conn_pool();
    tcp_cli_conn_t* mem;
    while (ax_atomic_i32_xchg(&CLIENT_POOL_LOCK, 1) == 1) { }
    mem = p ? (tcp_cli_conn_t*)ax_pool_alloc(p) : AX_NULL;
    ax_atomic_i32_store(&CLIENT_POOL_LOCK, 0);
    mem ? (mem->is_read = 1, 0) : 0;
    return mem;
}

void _destroy_tcp_client(tcp_cli_conn_t* t)
{
    ax_pool_t* p = _get_client_conn_pool();
    while (ax_atomic_i32_xchg(&CLIENT_POOL_LOCK, 1) == 1) { }
    p ? (ax_pool_free(p, t), 0) : 0;
    ax_atomic_i32_store(&CLIENT_POOL_LOCK, 0);
}
