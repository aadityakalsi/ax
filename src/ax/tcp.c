/*
For license details see ../../LICENSE
*/

/**
 * \file tcp.c
 * \date Aug 23, 2017
 */

#define AX_MIN_LOG_LEVEL AX_LOG_INFO
#include "ax/tcp.h"
#include "ax/assert.h"
#include "ax/log.h"

#include <uv.h>

#include <string.h>

/* tcp common */

AX_HIDDEN_TYPE(ax_sockaddr_t, 64);

AX_STATIC_ASSERT(sizeof(ax_sockaddr_t)>= sizeof(struct sockaddr_in6), ax_sockaddr_too_small);

static
int ax__init_loop_and_tcp(uv_loop_t* loop, uv_tcp_t* tcp, ax_sockaddr_t* saddr, ax_const_str addr, int port)
{
    int ret;
    struct addrinfo hints;
    uv_getaddrinfo_t addrinfo;
    char port_str[12];

    if ((ret = uv_loop_init(loop))) {
        AX_LOG(DBUG, "loop: %s\n", ax_error_str(ret));
        goto ax__init_loop_and_tcp_done;
    }

    if ((ret = uv_tcp_init(loop, tcp))) {
        AX_LOG(DBUG, "tcp_init: %s\n", ax_error_str(ret));
        goto ax__init_loop_and_tcp_done;
    }

    snprintf(port_str, 12, "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = 0;
    hints.ai_flags = AI_PASSIVE;
    if ((ret = uv_getaddrinfo(loop, &addrinfo, AX_NULL, addr, port_str, &hints))) {
        AX_LOG(DBUG, "getaddrinfo: %s\n", ax_error_str(ret));
        goto ax__init_loop_and_tcp_done;
    }
    memcpy(saddr, addrinfo.addrinfo->ai_addr, addrinfo.addrinfo->ai_addrlen);
    uv_freeaddrinfo(addrinfo.addrinfo);

ax__init_loop_and_tcp_done:
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

static
int ax__close_loop(uv_loop_t* loop)
{
    int ret = 0;
    if (!uv_loop_alive(loop)) {
        goto ax__close_loop_done;
    }
    uv_walk(loop, ax__close_all_handles, AX_NULL);
    if ((ret = uv_run(loop, UV_RUN_DEFAULT))) {
        AX_LOG(DBUG, "run_default: %s\n", ax_error_str(ret));
        goto ax__close_loop_done;
    }
    if ((ret = uv_loop_close(loop))) {
        AX_LOG(DBUG, "loop_close: %s\n", ax_error_str(ret));
        goto ax__close_loop_done;
    }

ax__close_loop_done:
    return ret;
}

/* tcp servers */

AX_STRUCT_TYPE(ax_tcp_srv_impl_t)
{
    uv_loop_t loop;
    uv_tcp_t server;
    ax_sockaddr_t saddr;
    ax_u32 state;
    ax_tcp_srv_cbk_t cbk;
};

#define ax_server_listening(x) ((x)->state & 1)
#define ax_server_set_listening(x,tf) ((x)->state = ((x)->state & ~((ax_u32)1)) | (tf ? 1 : 0))

AX_STATIC_ASSERT(sizeof(ax_tcp_srv_t) >= sizeof(ax_tcp_srv_impl_t), tcp_srv_type_too_small);

#define AX_SERVER_BACKLOG 128

static
void ax__server_on_connect(uv_stream_t* strm, int status)
{
    AX_LOG(DBUG, "tcp_srv_connect: %s\n", status ? ax_error_str(status) : "OK");
    ax_tcp_srv_impl_t* s = (ax_tcp_srv_impl_t*)(((ax_u8*)strm) - offsetof(ax_tcp_srv_impl_t, server));
    s->cbk.connect_fn(s->cbk.userdata, status);
    if (status) { return; }
    
}

static
int ax__server_ensure_listening(ax_tcp_srv_impl_t* s)
{
    int ret;
    if (ax_server_listening(s)) return 0;
    if ((ret = uv_listen((uv_stream_t*)&s->server, AX_SERVER_BACKLOG, ax__server_on_connect))) {
        AX_LOG(DBUG, "tcp_srv_listen: %s\n", ax_error_str(ret));
    } else {
        ax_server_set_listening(s, 1);
        s->cbk.listen_fn(s->cbk.userdata);
    }
    return ret;
}

int ax_tcp_srv_init_ip4(ax_tcp_srv_t* srv, ax_const_str addr, int port)
{
    int ret;
    ax_tcp_srv_impl_t* s = (ax_tcp_srv_impl_t*)srv;

    if ((ret = ax__init_loop_and_tcp(&s->loop, &s->server, &s->saddr, addr, port))) {
        goto ax_server_init_done;
    }

    if ((ret = uv_tcp_bind(&s->server, (struct sockaddr const*)&s->saddr, 0))) {
        AX_LOG(DBUG, "tcp_srv_bind: %s\n", ax_error_str(ret));
        goto ax_server_init_done;
    }

ax_server_init_done:
    s->state = 0;
    ax_server_set_listening(s, 0);
    return ret;
}

int ax_tcp_srv_destroy(ax_tcp_srv_t* srv)
{
    ax_tcp_srv_impl_t* s = (ax_tcp_srv_impl_t*)srv;
    int ret = ax__close_loop(&s->loop);
    ax_server_set_listening(s, 0);
    return ret;
}

void ax_tcp_srv_set_cbk(ax_tcp_srv_t* srv, ax_tcp_srv_cbk_t const* cbk)
{
    ((ax_tcp_srv_impl_t*)srv)->cbk = *cbk;
}

int ax_tcp_srv_start(ax_tcp_srv_t* srv)
{
    ax_tcp_srv_impl_t* s = (ax_tcp_srv_impl_t*)srv;
    int ret;
    ret = ax__server_ensure_listening(s);
    if (ret) return ret;
    ret = uv_run(&s->loop, UV_RUN_DEFAULT);
    if (ret) {
        AX_LOG(INFO, "tcp_srv_run: %s\n", ax_error_str(ret));
    }
    return ret;
}

void ax_tcp_srv_stop(ax_tcp_srv_t* srv)
{
    ax_tcp_srv_impl_t* s = (ax_tcp_srv_impl_t*)srv;
    uv_stop(&s->loop);
}

/* tcp clients */

AX_STRUCT_TYPE(ax_tcp_cli_impl_t)
{
    uv_loop_t loop;
    uv_tcp_t server;
    uv_connect_t conn;
    ax_sockaddr_t saddr;
    ax_u32 state;
    ax_tcp_cli_cbk_t cbk;
};

AX_STATIC_ASSERT(sizeof(ax_tcp_cli_t) >= sizeof(ax_tcp_cli_impl_t), tcp_cli_type_too_small);

#define ax_client_connected(x) ((x)->state & 1)
#define ax_client_set_connected(x,tf) ((x)->state = ((x)->state & ~((ax_u32)1)) | (tf ? 1 : 0))

static
void ax__client_on_connect(uv_connect_t* strm, int status)
{
    AX_LOG(DBUG, "tcp_cli_connect: %s\n", status ? ax_error_str(status) : "OK");
    ax_tcp_cli_impl_t* c = (ax_tcp_cli_impl_t*)(((ax_u8*)strm) - offsetof(ax_tcp_cli_impl_t, conn));
    c->cbk.connect_fn(c->cbk.userdata, status);
    if (status) { return; }
    
}

static
int ax__client_ensure_connected(ax_tcp_cli_impl_t* c)
{
    int ret;
    if (ax_client_connected(c)) return 0;
    if ((ret = uv_tcp_connect(&c->conn, &c->server, (struct sockaddr const*)&c->saddr, ax__client_on_connect))) {
        AX_LOG(DBUG, "tcp_cli_connect: %s\n", ax_error_str(ret));
    } else {
        ax_client_set_connected(c, 1);
    }
    return ret;
}

int ax_tcp_cli_init_ip4(ax_tcp_cli_t* cli, ax_const_str addr, int port)
{
    int ret;
    ax_tcp_cli_impl_t* c = (ax_tcp_cli_impl_t*)cli;

    if ((ret = ax__init_loop_and_tcp(&c->loop, &c->server, &c->saddr, addr, port))) {
        goto ax_client_init_done;
    }

    if ((ret = uv_tcp_keepalive(&c->server, 1, 60))) {
        AX_LOG(DBUG, "tcp_cli_keepalive: %s\n", ax_error_str(ret));
        goto ax_client_init_done;
    }

ax_client_init_done:
    c->state = 0;
    ax_client_set_connected(c, 0);
    return ret;
}

int ax_tcp_cli_destroy(ax_tcp_cli_t* cli)
{
    ax_tcp_cli_impl_t* c = (ax_tcp_cli_impl_t*)cli;
    int ret = ax__close_loop(&c->loop);
    ax_client_set_connected(c, 0);
    return ret;
}

void ax_tcp_cli_set_cbk(ax_tcp_cli_t* cli, ax_tcp_cli_cbk_t const* cbk)
{
    ((ax_tcp_cli_impl_t*)cli)->cbk = *cbk;
}

int ax_tcp_cli_connect(ax_tcp_cli_t* cli)
{
    ax_tcp_cli_impl_t* c = (ax_tcp_cli_impl_t*)cli;
    int ret;
    ret = ax__client_ensure_connected(c);
    if (ret) return ret;
    ret = uv_run(&c->loop, UV_RUN_DEFAULT);
    if (ret) {
        AX_LOG(INFO, "tcp_cli_run: %s\n", ax_error_str(ret));
    }
    return ret;
}

void ax_tcp_cli_stop(ax_tcp_cli_t* cli)
{
    ax_tcp_cli_impl_t* c = (ax_tcp_cli_impl_t*)cli;
    uv_stop(&c->loop);
}
