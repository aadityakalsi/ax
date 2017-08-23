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

AX_API
int ax_client_init_ip4(ax_client_t* cli, ax_const_str addr, int port);

AX_API
int ax_client_destroy(ax_client_t* cli);

AX_API
int ax_client_run_once(ax_client_t* cli);

AX_API
int ax_client_connect(ax_client_t* cli);

#endif/*_AX_CLIENT_H_*/
