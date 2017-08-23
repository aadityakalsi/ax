/*
For license details see ../../LICENSE
*/

/**
 * \file client.h
 * \date Aug 16, 2017
 */

#ifndef _AX_CLIENT_H_
#define _AX_CLIENT_H_

AX_HIDDEN_TYPE(ax_client_t, 1536);

AX_STRUCT_TYPE(ax_client_cbk_t)
{
    void* userdata;
    void (*connect_fn)(void* userdata, int status);
};

AX_API
int ax_client_init_ip4(ax_client_t* cli, ax_const_str addr, int port);

AX_API
int ax_client_destroy(ax_client_t* cli);

AX_API
void ax_client_set_callbacks(ax_client_t* cli, ax_client_cbk_t const* cbk);

AX_API
int ax_client_connect(ax_client_t* cli);

AX_API
void ax_client_stop(ax_client_t* cli);

#endif/*_AX_CLIENT_H_*/
