/*
For license details see ../../LICENSE
*/

/**
 * \file tcp-server.c
 * \date Aug 25, 2017
 */

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
#include "ax/atomic.h"

#include "ax/tcp-common.h"

#include <uv.h>

#include <string.h>

/* tcp servers */

AX_STRUCT_TYPE(ax_tcp_srv_impl_t)
{
    uv_loop_t loop;
    uv_tcp_t server;
    sockaddr_t saddr;
    ax_u32 state;
    ax_tcp_srv_cbk_t cbk;
};

#define CLI_BUFF_SIZE 4096

AX_STRUCT_TYPE(tcp_cli_conn_t)
{
    uv_tcp_t client;
    ax_tcp_srv_impl_t const* server;
    char buff[CLI_BUFF_SIZE];
    uv_write_t write;
};

#define srv_listening(x)        ((x)->state & 1)
#define srv_set_listening(x,tf) ((x)->state = ((x)->state & ~((ax_u32)1)) | (tf ? 1 : 0))

AX_STATIC_ASSERT(sizeof(ax_tcp_srv_t) >= sizeof(ax_tcp_srv_impl_t), tcp_srv_type_too_small);

#define AX_SERVER_BACKLOG 128

static ax_atomic_i32 CLIENT_POOL_LOCK = 0;

static
ax_pool_t* _get_client_conn_pool(void)
{
    static ax_pool_t CLI_CONN_POOL_STORAGE;
    static ax_pool_t* CLI_CONN_POOL = AX_NULL;
    if (!CLI_CONN_POOL) {
        while (ax_atomic_i32_xchg(&CLIENT_POOL_LOCK, 1) == 1) { }
        if (!CLI_CONN_POOL && ax_pool_init(&CLI_CONN_POOL_STORAGE, sizeof(tcp_cli_conn_t)) == 0) {
            CLI_CONN_POOL = &CLI_CONN_POOL_STORAGE;
        }
        ax_atomic_i32_store(&CLIENT_POOL_LOCK, 0);
    }
    return CLI_CONN_POOL;
}

static
tcp_cli_conn_t* _create_tcp_client(void)
{
    ax_pool_t* p = _get_client_conn_pool();
    tcp_cli_conn_t* mem;
    while (ax_atomic_i32_xchg(&CLIENT_POOL_LOCK, 1) == 1) { }
    mem = p ? ax_pool_alloc(p) : AX_NULL;
    ax_atomic_i32_store(&CLIENT_POOL_LOCK, 0);
    return mem;
}

static
void _destroy_tcp_client(tcp_cli_conn_t* t)
{
    ax_pool_t* p = _get_client_conn_pool();
    while (ax_atomic_i32_xchg(&CLIENT_POOL_LOCK, 1) == 1) { }
    p ? (ax_pool_free(p, t), 0) : 0;
    ax_atomic_i32_store(&CLIENT_POOL_LOCK, 0);
}

static
void _srv_on_alloc(uv_handle_t *h, size_t sugg_size, uv_buf_t *b)
{
    tcp_cli_conn_t* c = (tcp_cli_conn_t*)h;
    (void)sugg_size;
    b->base = c->buff;
    b->len = CLI_BUFF_SIZE;
}

static
void _srv_on_close(uv_handle_t* h)
{
    AX_LOG(DBUG, "tcp_srv_close_conn: (client %p)\n", h);
    _destroy_tcp_client((tcp_cli_conn_t*)h);
}

static void _srv_start_write(uv_stream_t* strm);

static
void _srv_write_stat(uv_write_t* w, int status)
{
    tcp_cli_conn_t* conn = BASE_PTR(w, tcp_cli_conn_t, write);
    ax_tcp_srv_impl_t const* s = conn->server;
    if (s->cbk.write_fn) {
        s->cbk.write_fn(s->cbk.userdata, status);
    }
    if (status) {
        AX_LOG(DBUG, "tcp_srv_write: %s\n", ax_error_str(status));
        return;
    }
    _srv_start_write((uv_stream_t*)&conn->client);
}

static void _srv_on_read(uv_stream_t* strm, ssize_t nread, uv_buf_t const* buf);

static
void _srv_start_write(uv_stream_t* strm)
{
    tcp_cli_conn_t* conn = BASE_PTR(strm, tcp_cli_conn_t, client);
    ax_tcp_srv_impl_t const* s = conn->server;
    ax_buf_t buf = { AX_STOP_WRITE_NO_READ, 0 };
    uv_buf_t uv_buf;

    if (s->cbk.write_data_fn) {
        s->cbk.write_data_fn(s->cbk.userdata, &buf);
    }

    if (buf.data == AX_STOP_WRITE_NO_READ) {
        uv_close((uv_handle_t*)strm, strm->close_cb);
    } else if (buf.data == AX_STOP_WRITE_START_READ) {
        uv_read_start(strm, _srv_on_alloc, _srv_on_read);
    } else {
        uv_buf.base = buf.data;
        uv_buf.len = (ax_sz)buf.len;
        uv_write(&conn->write, strm, &uv_buf, 1, _srv_write_stat);
    }
}

static
void _srv_on_read(uv_stream_t* strm, ssize_t nread, uv_buf_t const* buf)
{
    tcp_cli_conn_t* conn = BASE_PTR(strm, tcp_cli_conn_t, client);
    ax_tcp_srv_impl_t const* s = conn->server;
    ax_buf_t ax_buf;
    int status = nread > 0 ? 0 : (int)nread;

    if (!s->cbk.data_fn || nread < 0) {
        AX_LOG(DBUG, "tcp_srv_read_err: (client %p) %s\n", strm, status == 0 ? "OK" : ax_error_str(status));
        uv_close((uv_handle_t*)strm, _srv_on_close);
    }

    if (nread > 0) {
        AX_LOG(INFO, "data recd (client %p):\n--->|\n%.*s|<---\n", strm, (int)nread, buf->base);
        ax_buf.data = buf->base;
        ax_buf.len = (ax_i32)buf->len;
        if (s->cbk.data_fn) {
            s->cbk.data_fn(s->cbk.userdata, nread > 0 ? 0 : (int) nread, &ax_buf);
        }
        if (ax_buf.data == AX_STOP_READ_NO_WRITE) {
            uv_read_stop(strm);
            uv_close((uv_handle_t*)strm, _srv_on_close);
        } else if (ax_buf.data == AX_STOP_READ_START_WRITE) {
            uv_read_stop(strm);
            _srv_start_write(strm);
        }
    }
}

static
void _srv_on_connect(uv_stream_t* strm, int status)
{
    AX_LOG(DBUG, "tcp_srv_connect: %s\n", status ? ax_error_str(status) : "OK");
    ax_tcp_srv_impl_t* s = BASE_PTR(strm, ax_tcp_srv_impl_t, server);
    tcp_cli_conn_t* conn;

    if (s->cbk.connect_fn) {
        s->cbk.connect_fn(s->cbk.userdata, status);
    }
    if (status) { return; }

    conn = _create_tcp_client();
    if (!conn) {
        AX_LOG(DBUG, "tcp_srv_init_cli: %s\n", ax_error_str(AX_ENOMEM));
        return;
    }
    conn->server = s;
    if ((status = uv_tcp_init(&s->loop, &conn->client))) {
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
    conn->client.close_cb = _srv_on_close;
    uv_read_start((uv_stream_t *) (&conn->client), _srv_on_alloc, _srv_on_read);
}

static
int _srv_ensure_listening(ax_tcp_srv_impl_t* s)
{
    int ret;
    if (srv_listening(s)) return 0;
    if ((ret = uv_listen((uv_stream_t*)&s->server, AX_SERVER_BACKLOG, _srv_on_connect))) {
        AX_LOG(DBUG, "tcp_srv_listen: %s\n", ax_error_str(ret));
    } else {
        srv_set_listening(s, 1);
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

    if ((ret = _init_loop_and_tcp(&s->loop, &s->server, &s->saddr, addr, port))) {
        goto ax_server_init_done;
    }

    if ((ret = uv_tcp_bind(&s->server, (struct sockaddr const*)&s->saddr, 0))) {
        AX_LOG(DBUG, "tcp_srv_bind: %s\n", ax_error_str(ret));
        goto ax_server_init_done;
    }

ax_server_init_done:
    s->state = 0;
    srv_set_listening(s, 0);
    memset(&s->cbk, 0, sizeof(ax_tcp_srv_cbk_t));
    return ret;
}

int ax_tcp_srv_destroy(ax_tcp_srv_t* srv)
{
    ax_tcp_srv_impl_t* s = (ax_tcp_srv_impl_t*)srv;
    int ret = _close_loop(&s->loop);
    srv_set_listening(s, 0);
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
    ret = _srv_ensure_listening(s);
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
