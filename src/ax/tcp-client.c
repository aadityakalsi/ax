/*
For license details see ../../LICENSE
*/

/**
 * \file tcp-client.c
 * \date Aug 25, 2017
 */

#include "ax/tcp.h"
#include "ax/assert.h"
#include "ax/log.h"
#include "ax/pool.h"
#include "ax/atomic.h"

#include "ax/tcp-common.h"

#include <uv.h>

#include <string.h>

/* tcp clients */

AX_STRUCT_TYPE(ax_tcp_cli_impl_t)
{
    uv_loop_t loop;
    uv_tcp_t server;
    uv_connect_t conn;
    sockaddr_t saddr;
    uv_write_t write;
    ax_u32 state;
    ax_tcp_ctx_t ctx;
    ax_tcp_req_t req;
    uv_buf_t buf;
};

AX_STATIC_ASSERT(sizeof(ax_tcp_cli_t) >= sizeof(ax_tcp_cli_impl_t), tcp_cli_type_too_small);

#define cli_get_bit(x,b)   ((x)->state & (1 << b))
#define cli_set_bit(x,b,v) ((x)->state = ((x)->state & ~((ax_u32)(1 << b))) | (v ? (1 << b) : 0))

#define cli_connected(x)        cli_get_bit(x, 0)
#define cli_set_connected(x,tf) cli_set_bit(x, 0, tf)
#define cli_is_first(x)         cli_get_bit(x, 1)
#define cli_set_is_first(x,tf)  cli_set_bit(x, 1, tf)
#define cli_is_read(x)          cli_get_bit(x, 2)
#define cli_set_read(x,tf)      cli_set_bit(x, 2, tf)

static
void _cli_on_close(uv_handle_t* h)
{
    ax_tcp_cli_impl_t* c = BASE_PTR(h, ax_tcp_cli_impl_t, server);
    AX_LOG(DBUG, "cli_close: closing (client %p)\n", h);
    AX_ASSERT(c->ctx.destroy_req);
    c->ctx.destroy_req(c->ctx.state, &c->req);
    cli_set_connected(c, 0);
}

static
void _cli_set_buf(ax_tcp_cli_impl_t* c);

static
void _cli_on_alloc(uv_handle_t* h, size_t sugg_size, uv_buf_t* b)
{
    ax_tcp_cli_impl_t* c = BASE_PTR(h, ax_tcp_cli_impl_t, server);
    *b = c->buf;
}

static void _cli_start_write(uv_connect_t* conn);

static
void _cli_on_read(uv_stream_t* strm, ssize_t nread, uv_buf_t const* buf)
{
    ax_tcp_cli_impl_t* c = BASE_PTR(strm, ax_tcp_cli_impl_t, server);
    ax_buf_t ax_buf;
    int status = nread > 0 ? 0 : (int)nread;
    int next;

    ax_buf.data = buf->base;
    ax_buf.len = (ax_i32)buf->len;
    if (nread < 0) {
        AX_LOG(DBUG, "tcp_cli_read_err: (client %p) %s\n", strm, status == 0 ? "OK" : ax_error_str(status));
        next = -1;
    } else if (nread > 0) {
        AX_LOG(INFO, "data recd (client %p):\n--->|\n%.*s|<---\n", strm, (int)nread, buf->base);
        AX_ASSERT(c->req.read_cbk);
        c->req.read_cbk(c->req.req_ctx, status, &ax_buf);
        next = 1;
    } else {
        next = 0;
    }

    AX_ASSERT(c->req.free_read_buf);
    c->req.free_read_buf(c->req.req_ctx, &ax_buf);
    if (next == -1) {
        uv_close((uv_handle_t*)strm, _cli_on_close);
    } else if (next == 1) {
        _cli_set_buf(c);
    }
}


static void _cli_write_stat(uv_write_t* w, int status);

static
void _cli_start_write(uv_connect_t* conn)
{
    ax_tcp_cli_impl_t* c = BASE_PTR(conn, ax_tcp_cli_impl_t, conn);
    uv_write(&c->write, (uv_stream_t*)&c->server, &c->buf, 1, _cli_write_stat);
}

static
void _cli_write_stat(uv_write_t* w, int status)
{
    ax_tcp_cli_impl_t* c = BASE_PTR(w, ax_tcp_cli_impl_t, write);
    if (status) {
        AX_LOG(DBUG, "tcp_cli_write: %s\n", ax_error_str(status));
    }
    AX_ASSERT(c->req.write_cbk);
    c->req.write_cbk(c->req.req_ctx, status);

    _cli_set_buf(c);
    if (!cli_is_read(c) && !uv_is_closing((uv_handle_t const*)&c->server)) {
        _cli_start_write(&c->conn);
    }
}

static
void _cli_set_buf(ax_tcp_cli_impl_t* c)
{
    ax_buf_t buf;
    uv_stream_t* strm = (uv_stream_t*)&c->server;
    uv_handle_t* h = (uv_handle_t*)strm;
    if (cli_is_read(c)) {
        AX_ASSERT(c->req.get_read_buf);
        c->req.get_read_buf(c->req.req_ctx, &buf);
        if (buf.data == AX_STOP_READ_NO_WRITE) {
            if (!cli_is_first(c)) {
                uv_read_stop(strm);
            }
            uv_close(h, _cli_on_close);
        } else if (buf.data == AX_STOP_READ_START_WRITE) {
            if (!cli_is_first(c)) {
                uv_read_stop(strm);
            }
            cli_set_is_first(c, 0);
            cli_set_read(c, 0);
            _cli_set_buf(c);
            AX_ASSERT(!uv_is_closing(h));
            _cli_start_write(&c->conn);
        } else {
            c->buf.base = buf.data;
            c->buf.len = buf.len;
            cli_set_is_first(c, 0);
        }
    } else {
        AX_ASSERT(c->req.get_write_buf);
        c->req.get_write_buf(c->req.req_ctx, &buf);
        if (buf.data == AX_STOP_WRITE_NO_READ) {
            uv_close(h, _cli_on_close);
        } else if (buf.data == AX_STOP_WRITE_START_READ) {
            cli_set_is_first(c, 0);
            cli_set_read(c, 1);
            _cli_set_buf(c);
            AX_ASSERT(!uv_is_closing(h));
            uv_read_start(strm, _cli_on_alloc, _cli_on_read);
        } else {
            c->buf.base = buf.data;
            c->buf.len = buf.len;
            if (cli_is_first(c)) {
                uv_write(&c->write, strm, &c->buf, 1, _cli_write_stat);
            }
            cli_set_is_first(c, 0);
        }
    }
}

static
void _cli_on_connect(uv_connect_t* conn, int status)
{
    AX_LOG(DBUG, "tcp_cli_connect: %s (client %p)\n", status ? ax_error_str(status) : "OK", conn->handle);
    ax_tcp_cli_impl_t* c = BASE_PTR(conn, ax_tcp_cli_impl_t, conn);
    if (status) { return; }
    AX_ASSERT(c->ctx.init_req);
    c->ctx.init_req(c->ctx.state, &c->req);
    c->server.close_cb = _cli_on_close;
    cli_set_is_first(c, 1);
    cli_set_read(c, 0);
    _cli_set_buf(c);
}

static
int _cli_ensure_connected(ax_tcp_cli_impl_t* c)
{
    int ret;
    if (cli_connected(c)) return 0;
    if ((ret = uv_tcp_connect(&c->conn, &c->server, (struct sockaddr const*)&c->saddr, _cli_on_connect))) {
        AX_LOG(DBUG, "tcp_cli_connect: %s\n", ax_error_str(ret));
    } else {
        cli_set_connected(c, 1);
    }
    return ret;
}

int ax_tcp_cli_init_ip4(ax_tcp_cli_t* cli, ax_const_str addr, int port)
{
    int ret;
    ax_tcp_cli_impl_t* c = (ax_tcp_cli_impl_t*)cli;

    if ((ret = _init_loop_and_tcp(&c->loop, &c->server, &c->saddr, addr, port))) {
        goto ax_client_init_done;
    }

    if ((ret = uv_tcp_keepalive(&c->server, 1, 60))) {
        AX_LOG(DBUG, "tcp_cli_keepalive: %s\n", ax_error_str(ret));
        goto ax_client_init_done;
    }

ax_client_init_done:
    c->state = 0;
    cli_set_connected(c, 0);
    memset(&c->ctx, 0, sizeof(ax_tcp_ctx_t));
    return ret;
}

int ax_tcp_cli_destroy(ax_tcp_cli_t* cli)
{
    ax_tcp_cli_impl_t* c = (ax_tcp_cli_impl_t*)cli;
    int ret = _close_loop(&c->loop);
    cli_set_connected(c, 0);
    return ret;
}

void ax_tcp_cli_set_ctx(ax_tcp_cli_t* cli, ax_tcp_ctx_t const* ctx)
{
    ((ax_tcp_cli_impl_t*)cli)->ctx = *ctx;
}

int ax_tcp_cli_connect(ax_tcp_cli_t* cli)
{
    ax_tcp_cli_impl_t* c = (ax_tcp_cli_impl_t*)cli;
    int ret;
    ret = _cli_ensure_connected(c);
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