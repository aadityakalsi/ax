/*
For license details see ../../LICENSE
*/

/**
 * \file http.h
 * \date Sep 06, 2017
 */

#ifndef _AX_HTTP_H_
#define _AX_HTTP_H_

#include "ax/types.h"
#include "ax/tcp.h"

typedef struct ax_http_srv_impl ax_http_srv_impl_t;

AX_STRUCT_TYPE(ax_http_srv_t)
{
    ax_tcp_srv_t srv;
    ax_http_srv_impl_t* impl;
};

AX_STRUCT_TYPE(ax_http_req_t)
{
    ax_http_srv_t* srv;
    char remote_host[128];
    int remote_port;
    char method[64];
    int8_t method_len;
    char version[16];
    int8_t version_len;
    char path[1024];
    int16_t path_len;
    char path_decoded[1024];
    int16_t path_decoded_len;
    ax_str body;
    size_t body_len;
    void* data;
    void* _req_data;// private
};

AX_STRUCT_TYPE(ax_http_res_t)
{
    ax_i32 code;
    ax_str headers;
    ax_str extra_headers;
    void* data;
    void* _res_data;// private
};

AX_STRUCT_TYPE(ax_http_hdlr_t)
{
    void* data;
    void (*handle_req)(void* data, ax_http_req_t* req, ax_http_res_t* res);
    void (*after_write)(void* data, ax_http_req_t* req, ax_http_res_t* res, ax_buf_t const* buf);
    void (*destroy_hdlr)(void* data);
};

typedef enum ax_http_opt ax_http_opt_t;
enum ax_http_opt
{
    MaxRequestSize, /* default 8KB */
    MaxResponseHeader, /* default 1KB */
};

typedef union ax_http_opt_val ax_http_opt_val_t;
union ax_http_opt_val
{
    ax_str str;
    ax_i32 num;
};

AX_API
int ax_http_srv_init_ip4(ax_http_srv_t* srv, ax_const_str addr, int port);

AX_API
int ax_http_srv_set_opt(ax_http_srv_t* srv, ax_http_opt_t opt, ax_http_opt_val_t val);

AX_API
int ax_http_add_route(ax_http_srv_t* srv, ax_str route, ax_http_hdlr_t const* hdlr);

AX_API
int ax_http_add_static_dir_route(ax_http_srv_t* srv, ax_str path);

AX_API
int ax_http_serve_file(ax_http_req_t* req, ax_http_res_t* res, ax_str file);

AX_API
int ax_http_write_response(ax_http_hdlr_t* hdlr, ax_http_req_t* req, ax_http_res_t* res, ax_buf_t const* buf);

AX_API
int ax_http_srv_stop(ax_http_srv_t* srv);

AX_API
void ax_http_srv_destroy(ax_http_srv_t* srv);

#endif/*_AX_HTTP_H_*/
