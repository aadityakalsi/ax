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

#define AX_STOP_WRITE_NO_READ    AX_NULL
#define AX_STOP_WRITE_START_READ ((void*)-1)

#define AX_STOP_READ_NO_WRITE    AX_NULL
#define AX_STOP_READ_START_WRITE ((void*)-1)

/* tcp servers */

AX_HIDDEN_TYPE(ax_tcp_srv_t, 1536);

AX_STRUCT_TYPE(ax_tcp_srv_cbk_t)
{
    void* userdata;
    void (*listen_fn)(void* userdata);
    void (*connect_fn)(void* userdata, int status);
    void (*accept_fn)(void* userdata, ax_sz clientid);
    void (*data_fn)(void* userdata, int status, ax_buf_t* buf);
    void (*write_data_fn)(void* userdata, ax_buf_t* buf);
    void (*write_fn)(void* userdata, int status);
};

AX_API
int ax_tcp_srv_init_ip4(ax_tcp_srv_t* srv, ax_const_str addr, int port);

AX_API
int ax_tcp_srv_destroy(ax_tcp_srv_t* srv);

AX_API
void ax_tcp_srv_set_cbk(ax_tcp_srv_t* srv, ax_tcp_srv_cbk_t const* cbk);

AX_API
int ax_tcp_srv_start(ax_tcp_srv_t* srv);

AX_API
void ax_tcp_srv_stop(ax_tcp_srv_t* srv);

/* tcp clients */

AX_HIDDEN_TYPE(ax_tcp_cli_t, 5888);

AX_STRUCT_TYPE(ax_tcp_cli_cbk_t)
{
    void* userdata;
    void (*connect_fn)(void* userdata, int status);
    void (*data_fn)(void* userdata, int status, ax_buf_t* buf);
    void (*write_data_fn)(void* userdata, ax_buf_t*);
    void (*write_fn)(void* userdata, int status);
};

AX_API
int ax_tcp_cli_init_ip4(ax_tcp_cli_t* cli, ax_const_str addr, int port);

AX_API
int ax_tcp_cli_destroy(ax_tcp_cli_t* cli);

AX_API
void ax_tcp_cli_set_cbk(ax_tcp_cli_t* cli, ax_tcp_cli_cbk_t const* cbk);

AX_API
int ax_tcp_cli_connect(ax_tcp_cli_t* cli);

AX_API
void ax_tcp_cli_stop(ax_tcp_cli_t* cli);

#endif/*_AX_TCP_H_*/
