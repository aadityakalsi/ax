/*
For license details see ../../LICENSE
*/

/**
 * \file server.c
 * \date Aug 11, 2017
 */

#include "ax/assert.h"
#include "ax/server.h"

#define AX_MIN_LOG_LEVEL AX_LOG_INFO
#include "ax/log.h"

#include <uv.h>

#include <string.h>

AX_STRUCT_TYPE(ax_server_impl_t)
{
    uv_loop_t loop;
    uv_tcp_t server;
    ax_u32 state;
    ax_server_cbk_t cbk;
};

#define ax_server_listening(x) ((x)->state & 1)
#define ax_server_set_listening(x,tf) ((x)->state = ((x)->state & ~((ax_u32)1)) | (tf ? 1 : 0))

AX_STATIC_ASSERT(sizeof(ax_server_t) >= sizeof(ax_server_impl_t), type_too_small);

#define AX_SERVER_BACKLOG 128

static
void ax__server_on_connect(uv_stream_t* strm, int status)
{
    AX_LOG(DBUG, "connect: %s\n", status ? ax_error_str(status) : "OK");
    ax_server_impl_t* s = (ax_server_impl_t*)(((ax_u8*)strm) - offsetof(ax_server_impl_t, server));
    s->cbk.connect_fn(s->cbk.userdata, status);
    if (status) { return; }
    
}

static
int ax__server_ensure_listening(ax_server_impl_t* s)
{
    int ret;
    if (ax_server_listening(s)) return 0;
    if ((ret = uv_listen((uv_stream_t*)&s->server, AX_SERVER_BACKLOG, ax__server_on_connect))) {
        AX_LOG(DBUG, "listen: %s\n", ax_error_str(ret));
    } else {
        ax_server_set_listening(s, 1);
        s->cbk.listen_fn(s->cbk.userdata);
    }
    return ret;
}

static
void ax__close_all_handles(uv_handle_t* h, void* unused)
{
    AX_ASSERT(unused == AX_NULL);
    if (!uv_is_closing(h) && uv_is_active(h)) {
        uv_close(h, AX_NULL);
    }
}

int ax_server_init_ip4(ax_server_t* srv, ax_const_str addr, int port)
{
    int ret;
    char saddr[64];
    struct addrinfo hints;
    uv_getaddrinfo_t addrinfo;
    ax_server_impl_t* s = (ax_server_impl_t*)srv;
    char port_str[12];

    AX_STATIC_ASSERT(sizeof(saddr)>= sizeof(struct sockaddr_in6), saddr_too_small);

    if ((ret = uv_loop_init(&s->loop))) {
        AX_LOG(DBUG, "loop: %s\n", ax_error_str(ret));
        goto ax_server_init_done;
    }

    if ((ret = uv_tcp_init(&s->loop, &s->server))) {
        AX_LOG(DBUG, "tcp_init: %s\n", ax_error_str(ret));
        goto ax_server_init_done;
    }

    snprintf(port_str, 12, "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = 0;
    hints.ai_flags = AI_PASSIVE;
    if ((ret = uv_getaddrinfo(&s->loop, &addrinfo, AX_NULL, addr, port_str, &hints))) {
        AX_LOG(DBUG, "getaddrinfo: %s\n", ax_error_str(ret));
        goto ax_server_init_done;
    }
    memcpy(saddr, addrinfo.addrinfo->ai_addr, addrinfo.addrinfo->ai_addrlen);
    uv_freeaddrinfo(addrinfo.addrinfo);

    if ((ret = uv_tcp_bind(&s->server, (struct sockaddr const*)saddr, 0))) {
        AX_LOG(DBUG, "bind: %s\n", ax_error_str(ret));
        goto ax_server_init_done;
    }

ax_server_init_done:
    s->state = 0;
    ax_server_set_listening(s, 0);
    return ret;
}

int ax_server_destroy(ax_server_t* srv)
{
    ax_server_impl_t* s = (ax_server_impl_t*)srv;
    int ret = 0;

    if (!uv_loop_alive(&s->loop)) {
        goto ax_server_destroy_done;
    }

    uv_walk(&s->loop, ax__close_all_handles, AX_NULL);

    if ((ret = uv_run(&s->loop, UV_RUN_DEFAULT))) {
        AX_LOG(DBUG, "run_default: %s\n", ax_error_str(ret));
        goto ax_server_destroy_done;
    }

    if ((ret = uv_loop_close(&s->loop))) {
        AX_LOG(DBUG, "loop_close: %s\n", ax_error_str(ret));
        goto ax_server_destroy_done;
    }

ax_server_destroy_done:
    return ret;
}

void ax_server_set_callbacks(ax_server_t* srv, ax_server_cbk_t const* cbk)
{
    ((ax_server_impl_t*)srv)->cbk = *cbk;
}

int ax_server_start(ax_server_t* srv)
{
    ax_server_impl_t* s = (ax_server_impl_t*)srv;
    int ret;
    ret = ax__server_ensure_listening(s);
    if (ret) return ret;
    ret = uv_run(&s->loop, UV_RUN_DEFAULT);
    if (ret) {
        AX_LOG(INFO, "run_default: %s\n", ax_error_str(ret));
    }
    return ret;
}

void ax_server_stop(ax_server_t* srv)
{
    ax_server_impl_t* s = (ax_server_impl_t*)srv;
    uv_stop(&s->loop);
}
