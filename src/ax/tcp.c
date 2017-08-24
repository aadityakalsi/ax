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
#include "ax/pool.h"

#include <uv.h>

#include <string.h>

/* tcp common */

AX_HIDDEN_TYPE(ax_sockaddr_t, 64);

AX_STATIC_ASSERT(sizeof(ax_sockaddr_t) >= sizeof(struct sockaddr_in6), ax_sockaddr_too_small);

static
int __init_loop_and_tcp(uv_loop_t* loop, uv_tcp_t* tcp, ax_sockaddr_t* saddr, ax_const_str addr, int port)
{
    int ret;
    struct addrinfo hints;
    uv_getaddrinfo_t addrinfo;
    char port_str[12];

    if ((ret = uv_loop_init(loop))) {
        AX_LOG(DBUG, "loop: %s\n", ax_error_str(ret));
        goto __init_loop_and_tcp_done;
    }

    if ((ret = uv_tcp_init(loop, tcp))) {
        AX_LOG(DBUG, "tcp_init: %s\n", ax_error_str(ret));
        goto __init_loop_and_tcp_done;
    }

    snprintf(port_str, 12, "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = 0;
    hints.ai_flags = AI_PASSIVE;
    if ((ret = uv_getaddrinfo(loop, &addrinfo, AX_NULL, addr, port_str, &hints))) {
        AX_LOG(DBUG, "getaddrinfo: %s\n", ax_error_str(ret));
        goto __init_loop_and_tcp_done;
    }
    memcpy(saddr, addrinfo.addrinfo->ai_addr, addrinfo.addrinfo->ai_addrlen);
    uv_freeaddrinfo(addrinfo.addrinfo);

__init_loop_and_tcp_done:
    return ret;
}

static
void __close_all_handles(uv_handle_t* h, void* unused)
{
    AX_ASSERT(unused == AX_NULL);
    if (!uv_is_closing(h) && uv_is_active(h)) {
        uv_close(h, AX_NULL);
    }
}

static
int __close_loop(uv_loop_t* loop)
{
    int ret = 0;
    if (!uv_loop_alive(loop)) {
        goto __close_loop_done;
    }
    uv_walk(loop, __close_all_handles, AX_NULL);
    if ((ret = uv_run(loop, UV_RUN_DEFAULT))) {
        AX_LOG(DBUG, "run_default: %s\n", ax_error_str(ret));
        goto __close_loop_done;
    }
    if ((ret = uv_loop_close(loop))) {
        AX_LOG(DBUG, "loop_close: %s\n", ax_error_str(ret));
        goto __close_loop_done;
    }

__close_loop_done:
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

#define __srv_listening(x)        ((x)->state & 1)
#define __srv_set_listening(x,tf) ((x)->state = ((x)->state & ~((ax_u32)1)) | (tf ? 1 : 0))

AX_STATIC_ASSERT(sizeof(ax_tcp_srv_t) >= sizeof(ax_tcp_srv_impl_t), tcp_srv_type_too_small);

#define AX_SERVER_BACKLOG 128

static ax_pool_t TCP_SOCKET_POOL_STORAGE;
static ax_pool_t* TCP_SOCKET_POOL = AX_NULL;

static
ax_pool_t* __get_socket_pool(void)
{
    if (!TCP_SOCKET_POOL) {
        if (ax_pool_init(&TCP_SOCKET_POOL_STORAGE, sizeof(uv_tcp_t)) == 0) {
            TCP_SOCKET_POOL = &TCP_SOCKET_POOL_STORAGE;
        } else {
            return AX_NULL;
        }
    }
    return TCP_SOCKET_POOL;
}

static
uv_tcp_t* __create_tcp_socket(void)
{
    ax_pool_t* p = __get_socket_pool();
    return p ? ax_pool_alloc(p) : AX_NULL;
}

static
void __destroy_tcp_socket(uv_tcp_t* t)
{
    ax_pool_t* p = __get_socket_pool();
    p ? ax_pool_free(p, t), 0 : 0;
}

static ax_pool_t DATA_BUFFER_POOL_STORAGE;
static ax_pool_t* DATA_BUFFER_POOL = AX_NULL;

#define DATA_BUFFER_SIZE 8192

static
ax_pool_t* __get_buffer_pool(void)
{
    if (!DATA_BUFFER_POOL) {
        if (ax_pool_init(&DATA_BUFFER_POOL_STORAGE, DATA_BUFFER_SIZE) == 0) {
            DATA_BUFFER_POOL = &DATA_BUFFER_POOL_STORAGE;
        } else {
            return AX_NULL;
        }
    }
    return DATA_BUFFER_POOL;
}

static
ax_str __create_data_buffer(void)
{
    ax_pool_t* p = __get_buffer_pool();
    return p ? ax_pool_alloc(p) : AX_NULL;
}

static
void __destroy_data_buffer(ax_str b)
{
    ax_pool_t* p = __get_buffer_pool();
    p ? ax_pool_free(p, b), 0 : 0;
}

static
void __create_uv_buf(uv_handle_t* h, size_t sugg_size, uv_buf_t* b)
{
    // AX_LOG(DBUG, "sugg_size: %zu\n", sugg_size);
    // AX_ASSERT(sugg_size <= DATA_BUFFER_SIZE);
    b->base = __create_data_buffer();
    b->len = b->base ? DATA_BUFFER_SIZE : 0;
}

static
void __server_on_close(uv_handle_t* h)
{
    AX_LOG(DBUG, "tcp_srv_close_conn: (client %p)\n", h);
    __destroy_tcp_socket((uv_tcp_t*)h);
}

static
void __server_on_read(uv_stream_t* strm, ssize_t nread, uv_buf_t const* buf)
{
    if (nread > 0) {
        AX_LOG(INFO, "data: %.*s", (int)nread, buf->base);
    } else if (nread < 0) {
        AX_LOG(DBUG, "tcp_srv_read_err: (client %p) %s\n", strm, ax_error_str(nread));
        uv_close((uv_handle_t*)strm, __server_on_close);
    }

    __destroy_data_buffer(buf->base);
}

static
void __server_on_connect(uv_stream_t* strm, int status)
{
    AX_LOG(DBUG, "tcp_srv_connect: %s\n", status ? ax_error_str(status) : "OK");
    ax_tcp_srv_impl_t* s = (ax_tcp_srv_impl_t*)(((ax_u8*)strm) - offsetof(ax_tcp_srv_impl_t, server));
    uv_tcp_t* conn;

    if (s->cbk.connect_fn) {
        s->cbk.connect_fn(s->cbk.userdata, status);
    }
    if (status) { return; }

    conn = __create_tcp_socket();
    if (!conn) {
        AX_LOG(DBUG, "tcp_srv_init_cli: %s\n", ax_error_str(AX_ENOMEM));
        return;
    }
    if ((status = uv_tcp_init(&s->loop, conn))) {
        AX_LOG(DBUG, "tcp_srv_init_cli: %s\n", ax_error_str(status));
        return;
    }
    if ((status = uv_accept(strm, (uv_stream_t*)conn))) {
        AX_LOG(DBUG, "tcp_srv_accept: %s\n", ax_error_str(status));
        return;
    }
    AX_LOG(DBUG, "tcp_srv_open_conn: (client %p)\n", conn);
    if (s->cbk.accept_fn) {
        s->cbk.accept_fn(s->cbk.userdata, (ax_sz)conn);
    }
    uv_read_start((uv_stream_t*)conn, __create_uv_buf, __server_on_read);
}

static
int __server_ensure_listening(ax_tcp_srv_impl_t* s)
{
    int ret;
    if (__srv_listening(s)) return 0;
    if ((ret = uv_listen((uv_stream_t*)&s->server, AX_SERVER_BACKLOG, __server_on_connect))) {
        AX_LOG(DBUG, "tcp_srv_listen: %s\n", ax_error_str(ret));
    } else {
        __srv_set_listening(s, 1);
        if (s->cbk.listen_fn) {
            s->cbk.listen_fn(s->cbk.userdata);
        }
    }
    return ret;
}

int ax_tcp_srv_init_ip4(ax_tcp_srv_t* srv, ax_const_str addr, int port)
{
    int ret;
    ax_tcp_srv_impl_t* s = (ax_tcp_srv_impl_t*)srv;

    if ((ret = __init_loop_and_tcp(&s->loop, &s->server, &s->saddr, addr, port))) {
        goto ax_server_init_done;
    }

    if ((ret = uv_tcp_bind(&s->server, (struct sockaddr const*)&s->saddr, 0))) {
        AX_LOG(DBUG, "tcp_srv_bind: %s\n", ax_error_str(ret));
        goto ax_server_init_done;
    }

ax_server_init_done:
    s->state = 0;
    __srv_set_listening(s, 0);
    memset(&s->cbk, 0, sizeof(ax_tcp_srv_cbk_t));
    return ret;
}

int ax_tcp_srv_destroy(ax_tcp_srv_t* srv)
{
    ax_tcp_srv_impl_t* s = (ax_tcp_srv_impl_t*)srv;
    int ret = __close_loop(&s->loop);
    __srv_set_listening(s, 0);
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
    ret = __server_ensure_listening(s);
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

#define __cli_connected(x)        ((x)->state & 1)
#define __cli_set_connected(x,tf) ((x)->state = ((x)->state & ~((ax_u32)1)) | (tf ? 1 : 0))

static
void __client_on_connect(uv_connect_t* strm, int status)
{
    AX_LOG(DBUG, "tcp_cli_connect: %s\n", status ? ax_error_str(status) : "OK");
    ax_tcp_cli_impl_t* c = (ax_tcp_cli_impl_t*)(((ax_u8*)strm) - offsetof(ax_tcp_cli_impl_t, conn));
    if (c->cbk.connect_fn) {
        c->cbk.connect_fn(c->cbk.userdata, status);
    }
    if (status) { return; }
    
}

static
int __client_ensure_connected(ax_tcp_cli_impl_t* c)
{
    int ret;
    if (__cli_connected(c)) return 0;
    if ((ret = uv_tcp_connect(&c->conn, &c->server, (struct sockaddr const*)&c->saddr, __client_on_connect))) {
        AX_LOG(DBUG, "tcp_cli_connect: %s\n", ax_error_str(ret));
    } else {
        __cli_set_connected(c, 1);
    }
    return ret;
}

int ax_tcp_cli_init_ip4(ax_tcp_cli_t* cli, ax_const_str addr, int port)
{
    int ret;
    ax_tcp_cli_impl_t* c = (ax_tcp_cli_impl_t*)cli;

    if ((ret = __init_loop_and_tcp(&c->loop, &c->server, &c->saddr, addr, port))) {
        goto ax_client_init_done;
    }

    if ((ret = uv_tcp_keepalive(&c->server, 1, 60))) {
        AX_LOG(DBUG, "tcp_cli_keepalive: %s\n", ax_error_str(ret));
        goto ax_client_init_done;
    }

ax_client_init_done:
    c->state = 0;
    __cli_set_connected(c, 0);
    memset(&c->cbk, 0, sizeof(ax_tcp_cli_cbk_t));
    return ret;
}

int ax_tcp_cli_destroy(ax_tcp_cli_t* cli)
{
    ax_tcp_cli_impl_t* c = (ax_tcp_cli_impl_t*)cli;
    int ret = __close_loop(&c->loop);
    __cli_set_connected(c, 0);
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
    ret = __client_ensure_connected(c);
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
