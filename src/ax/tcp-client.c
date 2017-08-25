/*
For license details see ../../LICENSE
*/

/**
 * \file tcp-client.c
 * \date Aug 25, 2017
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

/* tcp clients */

#define CLI_BUFF_SIZE 4096

AX_STRUCT_TYPE(ax_tcp_cli_impl_t)
{
    uv_loop_t loop;
    uv_tcp_t server;
    uv_connect_t conn;
    sockaddr_t saddr;
    uv_write_t write;
    char buff[CLI_BUFF_SIZE];
    ax_u32 state;
    ax_tcp_cli_cbk_t cbk;
};

AX_STATIC_ASSERT(sizeof(ax_tcp_cli_t) >= sizeof(ax_tcp_cli_impl_t), tcp_cli_type_too_small);

#define cli_connected(x)        ((x)->state & 1)
#define cli_set_connected(x,tf) ((x)->state = ((x)->state & ~((ax_u32)1)) | (tf ? 1 : 0))

static
void _cli_close_conn(uv_handle_t* h)
{
    AX_LOG(DBUG, "cli_close: closing (client %p)\n", h);
}

static
void _cli_on_alloc(uv_handle_t* h, size_t sugg_size, uv_buf_t* b)
{
    uv_connect_t* conn = (uv_connect_t*)h->data;
    ax_tcp_cli_impl_t* c = BASE_PTR(conn, ax_tcp_cli_impl_t, conn);
    (void)sugg_size;
    b->base = c->buff;
    b->len = CLI_BUFF_SIZE;
}

static void _cli_start_write(uv_connect_t* conn);

static
void _cli_on_read(uv_stream_t* strm, ssize_t nread, uv_buf_t const* buf)
{
    uv_connect_t* conn = (uv_connect_t*)strm->data;
    ax_tcp_cli_impl_t* c = BASE_PTR(conn, ax_tcp_cli_impl_t, conn);
    ax_buf_t ax_buf;
    int status = (int)nread;

    if (!c->cbk.data_fn || nread < 0) {
        AX_LOG(DBUG, "tcp_srv_read_err: (client %p) %s\n", strm, ax_error_str(status));
        uv_close((uv_handle_t*)strm, _cli_close_conn);
    }

    if (nread > 0) {
        AX_LOG(INFO, "data recd (client %p):\n--->|\n%.*s|<---\n", strm, (int)nread, buf->base);
        ax_buf.data = buf->base;
        ax_buf.len = (ax_i32)buf->len;
        c->cbk.data_fn(c->cbk.userdata, nread > 0 ? 0 : (int)nread, &ax_buf);
        if (ax_buf.data == AX_STOP_READ_NO_WRITE) {
            uv_read_stop(strm);
            uv_close((uv_handle_t*)strm, _cli_close_conn);
        } else if (ax_buf.data == AX_STOP_READ_START_WRITE) {
            uv_read_stop(strm);
            _cli_start_write(conn);
        }
    }
}


static void _cli_write_stat(uv_write_t* w, int status);

static
void _cli_start_write(uv_connect_t* conn)
{
    ax_tcp_cli_impl_t* c = BASE_PTR(conn, ax_tcp_cli_impl_t, conn);
    ax_buf_t buf = { AX_NULL, 0 };
    uv_buf_t uv_buf;
    if (c->cbk.write_data_fn) {
        c->cbk.write_data_fn(c->cbk.userdata, &buf);
    }
    if (buf.data == AX_STOP_WRITE_NO_READ) {
        uv_close((uv_handle_t*)conn->handle, _cli_close_conn);
    } else if (buf.data == AX_STOP_WRITE_START_READ) {
        conn->handle->data = conn;
        uv_read_start(conn->handle, _cli_on_alloc, _cli_on_read);
    } else {
        uv_buf.base = buf.data;
        uv_buf.len = (ax_sz)buf.len;
        uv_write(&c->write, conn->handle, &uv_buf, 1, _cli_write_stat);
    }
}

static
void _cli_write_stat(uv_write_t* w, int status)
{
    ax_tcp_cli_impl_t* c = BASE_PTR(w, ax_tcp_cli_impl_t, write);
    if (c->cbk.write_fn) {
        c->cbk.write_fn(c->cbk.userdata, status);
    }
    if (status) {
        AX_LOG(DBUG, "tcp_cli_write: %s\n", ax_error_str(status));
        return;
    }
    _cli_start_write(&c->conn);
}

static
void _cli_on_connect(uv_connect_t* conn, int status)
{
    AX_LOG(DBUG, "tcp_cli_connect: %s (client %p)\n", status ? ax_error_str(status) : "OK", conn->handle);
    ax_tcp_cli_impl_t* c = BASE_PTR(conn, ax_tcp_cli_impl_t, conn);
    if (c->cbk.connect_fn) {
        c->cbk.connect_fn(c->cbk.userdata, status);
    }
    if (status) { return; }
    _cli_start_write(conn);
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
    memset(&c->cbk, 0, sizeof(ax_tcp_cli_cbk_t));
    return ret;
}

int ax_tcp_cli_destroy(ax_tcp_cli_t* cli)
{
    ax_tcp_cli_impl_t* c = (ax_tcp_cli_impl_t*)cli;
    int ret = _close_loop(&c->loop);
    cli_set_connected(c, 0);
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
