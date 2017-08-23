/*
For license details see ../../LICENSE
*/

/**
 * \file client.c
 * \date Aug 16, 2017
 */

#include "ax/assert.h"
#include "ax/client.h"

#define AX_MIN_LOG_LEVEL AX_LOG_INFO
#include "ax/log.h"

#include <uv.h>

#include <string.h>

AX_STRUCT_TYPE(ax_client_impl_t)
{
    uv_loop_t loop;
    uv_tcp_t server;
    uv_connect_t conn;
    char saddr[64];
    ax_u32 state;
    ax_client_cbk_t cbk;
};

AX_STATIC_ASSERT(sizeof(ax_client_t) >= sizeof(ax_client_impl_t), type_too_small);

#define ax_client_connected(x) ((x)->state & 1)
#define ax_client_set_connected(x,tf) ((x)->state = ((x)->state & ~((ax_u32)1)) | (tf ? 1 : 0))

static
void ax__client_on_connect(uv_connect_t* strm, int status)
{
    AX_LOG(DBUG, "connect: %s\n", status ? uv_strerror(status) : "OK");
    ax_client_impl_t* c = (ax_client_impl_t*)(((ax_u8*)strm) - offsetof(ax_client_impl_t, conn));
    c->cbk.connect_fn(c->cbk.userdata, status);
    if (status) { return; }
    
}

static
int ax__client_ensure_connected(ax_client_impl_t* c)
{
    int ret;
    if (ax_client_connected(c)) return 0;
    if ((ret = uv_tcp_connect(&c->conn, &c->server, (struct sockaddr const*)&c->saddr, ax__client_on_connect))) {
        AX_LOG(DBUG, "connect: %s\n", uv_strerror(ret));
    } else {
        ax_client_set_connected(c, 1);
    }
    return ret;
}

static
void ax__close_all_handles(uv_handle_t* h, void* unused)
{
    AX_ASSERT(unused == AX_NULL);
    if (!uv_is_closing(h) && uv_is_active(h)) {
        uv_close(h, AX_NULL);
        if (uv_has_ref(h)) {
            uv_unref(h);
        }
    }
}

int ax_client_init_ip4(ax_client_t* cli, ax_const_str addr, int port)
{
    int ret;
    char saddr[64];
    struct addrinfo hints;
    uv_getaddrinfo_t addrinfo;
    ax_client_impl_t* c = (ax_client_impl_t*)cli;
    char port_str[12];

    AX_STATIC_ASSERT(sizeof(saddr)>= sizeof(struct sockaddr_in6), saddr_too_small);

    if ((ret = uv_loop_init(&c->loop))) {
        AX_LOG(DBUG, "loop: %s\n", uv_strerror(ret));
        goto ax_client_init_done;
    }

    if ((ret = uv_tcp_init(&c->loop, &c->server))) {
        AX_LOG(DBUG, "tcp_init: %s\n", uv_strerror(ret));
        goto ax_client_init_done;
    }

    snprintf(port_str, 12, "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = 0;
    hints.ai_flags = AI_PASSIVE;
    if ((ret = uv_getaddrinfo(&c->loop, &addrinfo, AX_NULL, addr, port_str, &hints))) {
        AX_LOG(DBUG, "getaddrinfo: %s\n", uv_strerror(ret));
        goto ax_client_init_done;
    }
    memcpy(&c->saddr, addrinfo.addrinfo->ai_addr, addrinfo.addrinfo->ai_addrlen);
    uv_freeaddrinfo(addrinfo.addrinfo);

    if ((ret = uv_tcp_keepalive(&c->server, 1, 60))) {
        AX_LOG(DBUG, "keepalive: %s\n", uv_strerror(ret));
        goto ax_client_init_done;
    }

ax_client_init_done:
    c->state = 0;
    ax_client_set_connected(c, 0);
    return ret;
}

int ax_client_destroy(ax_client_t* cli)
{
    ax_client_impl_t* c = (ax_client_impl_t*)cli;
    int ret = 0;

    if (!uv_loop_alive(&c->loop)) {
        goto ax_client_destroy_done;
    }

    uv_walk(&c->loop, ax__close_all_handles, AX_NULL);

    if ((ret = uv_run(&c->loop, UV_RUN_DEFAULT))) {
        AX_LOG(DBUG, "loop_run: %s\n", uv_strerror(ret));
        goto ax_client_destroy_done;
    }

    if ((ret = uv_loop_close(&c->loop))) {
        AX_LOG(DBUG, "loop_close: %s\n", uv_strerror(ret));
        goto ax_client_destroy_done;
    }

    ax_client_set_connected(c, 0);

ax_client_destroy_done:
    return ret;
}

void ax_client_set_callbacks(ax_client_t* cli, ax_client_cbk_t const* cbk)
{
    ((ax_client_impl_t*)cli)->cbk = *cbk;
}

int ax_client_run_once(ax_client_t* cli)
{
    ax_client_impl_t* c = (ax_client_impl_t*)cli;
    int ret;
    ret = ax__client_ensure_connected(c);
    if (ret) return ret;
    ret = uv_run(&c->loop, UV_RUN_ONCE);
    if (ret) {
        AX_LOG(INFO, "loop_run_once: %s\n", uv_strerror(ret));
    }
    return ret;
}

int ax_client_connect(ax_client_t* cli)
{
    ax_client_impl_t* c = (ax_client_impl_t*)cli;
    int ret;
    ret = ax__client_ensure_connected(c);
    if (ret) return ret;
    ret = uv_run(&c->loop, UV_RUN_DEFAULT);
    if (ret) {
        AX_LOG(INFO, "loop_run_once: %s\n", uv_strerror(ret));
    }
    return ret;
}
