/*
For license details see ../../LICENSE
*/

/**
 * \file server.h
 * \date Aug 11, 2017
 */

#ifndef _AX_SERVER_H_
#define _AX_SERVER_H_

#include "ax/types.h"

AX_HIDDEN_TYPE(ax_server_t, 1536);

AX_STRUCT_TYPE(ax_req_t)
{
};

AX_STRUCT_TYPE(ax_server_cbk_t)
{
    void* userdata;
    void (*listen_fn)(void* userdata);
    void (*connect_fn)(void* userdata, int status);
};

AX_API
int ax_server_init_ip4(ax_server_t* srv, ax_const_str addr, int port);

AX_API
int ax_server_destroy(ax_server_t* srv);

AX_API
void ax_server_set_callbacks(ax_server_t* srv, ax_server_cbk_t const* cbk);

AX_API
int ax_server_run_once(ax_server_t* srv);

AX_API
int ax_server_start(ax_server_t* srv);

#endif/*_AX_SERVER_H_*/
