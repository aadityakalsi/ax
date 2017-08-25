/*
For license details see ../../LICENSE
*/

/**
 * \file tcp-common.h
 * \date Aug 25, 2017
 */

#ifndef _AX_TCP_COMMON_H_
#define _AX_TCP_COMMON_H_

#include "ax/types.h"

#include <uv.h>

AX_HIDDEN_TYPE(sockaddr_t, 64);

int _init_loop_and_tcp(uv_loop_t* loop, uv_tcp_t* tcp, sockaddr_t* saddr, ax_const_str addr, int port);

void _close_all_handles(uv_handle_t* h, void* unused);

int _close_loop(uv_loop_t* loop);

#endif/*_AX_TCP_COMMON_H_*/
