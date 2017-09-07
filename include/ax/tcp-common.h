/*
For license details see ../../LICENSE
*/

/**
 * \file tcp-common.h
 * \date Aug 25, 2017
 */

#ifndef _AX_TCP_COMMON_H_
#define _AX_TCP_COMMON_H_

#include "ax/tcp.h"
#include "ax/types.h"

#include <uv.h>

AX_HIDDEN_TYPE(sockaddr_t, 64);

int _init_loop_and_tcp(uv_loop_t* loop, uv_tcp_t* tcp, sockaddr_t* saddr, ax_const_str addr, int port);

void _close_all_handles(uv_handle_t* h, void* unused);

int _close_loop(uv_loop_t* loop);

typedef struct _ax_tcp_srv_impl_t ax_tcp_srv_impl_t;

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

tcp_cli_conn_t* _create_tcp_client(void);

void _destroy_tcp_client(tcp_cli_conn_t* t);

#endif/*_AX_TCP_COMMON_H_*/
