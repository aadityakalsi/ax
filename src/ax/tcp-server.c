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
    ax_tcp_ctx_t ctx;
};

AX_STRUCT_TYPE(tcp_cli_conn_t)
{
    uv_tcp_t client;
    ax_tcp_srv_impl_t const* server;
    uv_write_t write;
    ax_tcp_req_t req;
    uv_buf_t buf;
    ax_u32 is_first : 1;
    ax_u32 is_read : 1;
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
    mem = p ? (tcp_cli_conn_t*)ax_pool_alloc(p) : AX_NULL;
    ax_atomic_i32_store(&CLIENT_POOL_LOCK, 0);
    mem ? (mem->is_read = 1, 0) : 0;
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

/* server api */

static
void _srv_on_alloc(uv_handle_t* h, size_t sugg_sz, uv_buf_t* b)
{
    tcp_cli_conn_t* conn = BASE_PTR(h, tcp_cli_conn_t, client);
    *b = conn->buf;
}

static
void _srv_on_read(uv_stream_t* strm, ssize_t nread, uv_buf_t const* buf);

static
void _srv_start_write(uv_stream_t* strm);

static
void _srv_on_close(uv_handle_t* h);

inline static
void _srv_set_buf(tcp_cli_conn_t* conn)
{
    ax_buf_t buf = {0, 0};
    uv_stream_t* strm = (uv_stream_t*)&conn->client;
    uv_handle_t* h = (uv_handle_t*)strm;
    if (conn->is_read) {
        AX_ASSERT(conn->req.get_read_buf);
        conn->req.get_read_buf(conn->req.req_ctx, &buf);
        if (buf.data == AX_STOP_READ_NO_WRITE) {
            if (!conn->is_first) {
                uv_read_stop(strm);
            }
            uv_close(h, _srv_on_close);
        } else if (buf.data == AX_STOP_READ_START_WRITE) {
            if (!conn->is_first) {
                uv_read_stop(strm);
            }
            conn->is_first = 0;
            conn->is_read = 0;
            _srv_set_buf(conn);
            AX_ASSERT(!uv_is_closing(h));
            _srv_start_write(strm);
        } else {
            conn->buf.base = buf.data;
            conn->buf.len = buf.len;
            if (conn->is_first) {
                uv_read_start(strm, _srv_on_alloc, _srv_on_read);
            }
            conn->is_first = 0;
        }
    } else {
        AX_ASSERT(conn->req.get_write_buf);
        conn->req.get_write_buf(conn->req.req_ctx, &buf);
        if (buf.data == AX_STOP_WRITE_NO_READ) {
            uv_close(h, _srv_on_close);
        } else if (buf.data == AX_STOP_WRITE_START_READ) {
            conn->is_first = 0;
            conn->is_read = 1;
            _srv_set_buf(conn);
            AX_ASSERT(!uv_is_closing(h));
            uv_read_start(strm, _srv_on_alloc, _srv_on_read);
        } else {
            conn->is_first = 0;
            conn->buf.base = buf.data;
            conn->buf.len = buf.len;
        }
    }
}

static
void _srv_on_close(uv_handle_t* h)
{
    tcp_cli_conn_t* conn = BASE_PTR(h, tcp_cli_conn_t, client);
    ax_tcp_srv_impl_t const* s = conn->server;
    AX_LOG(DBUG, "tcp_srv_close_conn: (client %p)\n", h);
    AX_ASSERT(s->ctx.destroy_req);
    s->ctx.destroy_req(s->ctx.state, &conn->req);
    _destroy_tcp_client((tcp_cli_conn_t*)h);
}

static
void _srv_write_stat(uv_write_t* w, int status)
{
    tcp_cli_conn_t* conn = BASE_PTR(w, tcp_cli_conn_t, write);
    ax_buf_t buf = { conn->buf.base, conn->buf.len };
    AX_ASSERT(conn->req.free_write_buf);
    conn->req.free_write_buf(conn->req.req_ctx, &buf);
    if (status) {
        AX_LOG(DBUG, "tcp_srv_write: %s\n", ax_error_str(status));
    }
    AX_ASSERT(conn->req.write_cbk);
    conn->req.write_cbk(conn->req.req_ctx, status);

    _srv_set_buf(conn);
    if (!conn->is_read && !uv_is_closing((uv_handle_t const*)&conn->client)) {
        _srv_start_write((uv_stream_t*)&conn->client);
    }
}

static
void _srv_start_write(uv_stream_t* strm)
{
    tcp_cli_conn_t* conn = BASE_PTR(strm, tcp_cli_conn_t, client);
    uv_write(&conn->write, strm, &conn->buf, 1, _srv_write_stat);
}

static
void _srv_on_read(uv_stream_t* strm, ssize_t nread, uv_buf_t const* buf)
{
    tcp_cli_conn_t* conn = BASE_PTR(strm, tcp_cli_conn_t, client);
    ax_tcp_srv_impl_t const* s = conn->server;
    ax_buf_t ax_buf;
    int status = nread > 0 ? 0 : (int)nread;
    int next;

    ax_buf.data = buf->base;
    ax_buf.len = (ax_i32)nread;
    if (nread < 0) {
        AX_LOG(DBUG, "tcp_srv_read_err: (client %p) %s\n", strm, status == 0 ? "OK" : ax_error_str(status));
        next = -1;
    } else if (nread > 0) {
        AX_LOG(INFO, "data recd (client %p):\n--->|\n%.*s|<---\n", strm, (int)nread, buf->base);
        AX_ASSERT(conn->req.read_cbk);
        conn->req.read_cbk(conn->req.req_ctx, status, &ax_buf);
        next = 1;
    } else {
        next = 0;
    }

    AX_ASSERT(conn->req.free_read_buf);
    conn->req.free_read_buf(conn->req.req_ctx, &ax_buf);
    if (next == -1) {
        uv_close((uv_handle_t*)strm, _srv_on_close);
    } else if (next == 1) {
        _srv_set_buf(conn);
    }
}

static
void _srv_on_connect(uv_stream_t* strm, int status)
{
    AX_LOG(DBUG, "tcp_srv_connect: %s\n", status ? ax_error_str(status) : "OK");
    ax_tcp_srv_impl_t* s = BASE_PTR(strm, ax_tcp_srv_impl_t, server);
    tcp_cli_conn_t* conn;

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

    AX_ASSERT(s->ctx.init_req);
    s->ctx.init_req(s->ctx.state, &conn->req);

    conn->client.close_cb = _srv_on_close;
    conn->is_first = 1;
    _srv_set_buf(conn);
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
    memset(&s->ctx, 0, sizeof(ax_tcp_ctx_t));
    return ret;
}

int ax_tcp_srv_destroy(ax_tcp_srv_t* srv)
{
    ax_tcp_srv_impl_t* s = (ax_tcp_srv_impl_t*)srv;
    int ret = _close_loop(&s->loop);
    srv_set_listening(s, 0);
    return ret;
}

void ax_tcp_srv_set_ctx(ax_tcp_srv_t* srv, ax_tcp_ctx_t const* ctx)
{
    ((ax_tcp_srv_impl_t*)srv)->ctx = *ctx;
}

typedef struct outband_write_s outband_write_t;
struct outband_write_s
{
    uv_write_t write;
    uv_buf_t buf;
    ax_tcp_req_t* req;
    void (*free_write_buf)(void* req_ctx, ax_buf_t const* buf);
    void (*write_cbk)(void* req_ctx, int err);
};

AX_STATIC_ASSERT(sizeof(outband_write_t) <= sizeof(tcp_cli_conn_t), outband_t_too_large);

static
void outband_write_cbk(uv_write_t* write, int err)
{
    outband_write_t* wreq = BASE_PTR(write, outband_write_t, write);
    ax_buf_t buf;
    if (err) {
        AX_LOG(DBUG, "tcp_srv_write: %s\n", ax_error_str(err));
    }
    buf.data = wreq->buf.base;
    buf.len = (int)wreq->buf.len;
    wreq->free_write_buf(wreq->req->req_ctx, &buf);
    wreq->write_cbk(wreq->req->req_ctx, err);
    _destroy_tcp_client((tcp_cli_conn_t*)wreq);
}

void ax_tcp_srv_write(ax_tcp_srv_t* srv, ax_tcp_req_t* req, ax_buf_t const* buf,
                      void (*free_write_buf)(void* req_ctx, ax_buf_t const* buf),
                      void (*write_cbk)(void* req_ctx, int err))
{
    ax_tcp_srv_impl_t* s = (ax_tcp_srv_impl_t*)srv;
    tcp_cli_conn_t* cli = BASE_PTR(req, tcp_cli_conn_t, req);
    outband_write_t* wreq;
    AX_ASSERT(srv_listening(s));
    wreq = (outband_write_t*)_create_tcp_client();
    AX_ASSERT(wreq);
    wreq->req = req;
    wreq->buf.base = buf->data;
    wreq->buf.len = (size_t)buf->len;
    wreq->free_write_buf = free_write_buf;
    wreq->write_cbk = write_cbk;
    uv_write(&wreq->write, (uv_stream_t*)&cli->client, &wreq->buf, 1, outband_write_cbk);
}

int ax_tcp_srv_start(ax_tcp_srv_t* srv)
{
    ax_tcp_srv_impl_t* s = (ax_tcp_srv_impl_t*)srv;
    int ret;
    ret = _srv_ensure_listening(s);
    if (ret) return ret;
    AX_ASSERT(s->ctx.on_start);
    s->ctx.on_start(s->ctx.state);
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
