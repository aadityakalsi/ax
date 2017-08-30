/*
For license details see ../../LICENSE
*/

/**
 * \file tcp.h
 * \date Aug 23, 2017
 */

#ifndef _AX_TCP_H_
#define _AX_TCP_H_

#include "ax/types.h"

/* buffer type */

AX_STRUCT_TYPE(ax_buf_t)
{
    ax_str data;
    ax_i32 len;
};

/* set ax_buf_t::data to these constants to change behaviour */
#define AX_STOP_WRITE_NO_READ    AX_NULL
#define AX_STOP_WRITE_START_READ ((void*)-1)

#define AX_STOP_READ_NO_WRITE    AX_NULL
#define AX_STOP_READ_START_WRITE ((void*)-1)

/* tcp servers */

/* by default, calls ax_tcp_req_t::get_read_buf first */
AX_STRUCT_TYPE(ax_tcp_req_t)
{
    void* req_ctx;
    /* modify buf->data to change behaviour. see AX_STOP* above */
    void (*get_read_buf)(void* req_ctx, ax_buf_t* buf);
    void (*free_read_buf)(void* req_ctx, ax_buf_t const* buf);
    /* modify buf->data to change behaviour. see AX_STOP* above */
    void (*get_write_buf)(void* req_ctx, ax_buf_t* buf);
    void (*free_write_buf)(void* req_ctx, ax_buf_t const* buf);
    /* when operation is performed */
    void (*read_cbk)(void* req_ctx, int err, ax_buf_t const* buf);
    void (*write_cbk)(void* req_ctx, int err);
};

AX_STRUCT_TYPE(ax_tcp_ctx_t)
{
    void* state;
    void (*on_start)(void* state);
    void (*init_req)(void* state, ax_tcp_req_t* req);
    void (*destroy_req)(void* state, ax_tcp_req_t* req);
};

AX_HIDDEN_TYPE(ax_tcp_srv_t, 1536);

AX_API
int ax_tcp_srv_init_ip4(ax_tcp_srv_t* srv, ax_const_str addr, int port);

AX_API
int ax_tcp_srv_destroy(ax_tcp_srv_t* srv);

AX_API
void ax_tcp_srv_set_ctx(ax_tcp_srv_t* srv, ax_tcp_ctx_t const* ctx);

AX_API
int ax_tcp_srv_start(ax_tcp_srv_t* srv);

AX_API
void ax_tcp_srv_stop(ax_tcp_srv_t* srv);

/* tcp clients */

AX_HIDDEN_TYPE(ax_tcp_cli_t, 1792);

AX_API
int ax_tcp_cli_init_ip4(ax_tcp_cli_t* cli, ax_const_str addr, int port);

AX_API
int ax_tcp_cli_destroy(ax_tcp_cli_t* cli);

AX_API
void ax_tcp_cli_set_ctx(ax_tcp_cli_t* cli, ax_tcp_ctx_t const* ctx);

AX_API
int ax_tcp_cli_connect(ax_tcp_cli_t* cli);

AX_API
void ax_tcp_cli_stop(ax_tcp_cli_t* cli);

#endif/*_AX_TCP_H_*/
