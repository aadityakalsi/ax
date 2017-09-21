/*
For license details see ../../LICENSE
*/

/**
 * \file http.c
 * \date Sep 06, 2017
 */

#include "ax/http.h"

struct ax_http_srv_impl
{
    void* m;
};

int ax_http_srv_init_ip4(ax_http_srv_t* srv, ax_const_str addr, int port)
{
    return 0;
}

int ax_http_srv_set_opt(ax_http_srv_t* srv, ax_http_opt_t opt, ax_http_opt_val_t val)
{
    return 0;
}

int ax_http_add_route(ax_http_srv_t* srv, ax_str route, ax_http_hdlr_t const* hdlr)
{
    return 0;
}

int ax_http_add_static_dir_route(ax_http_srv_t* srv, ax_str path)
{
    return 0;
}

int ax_http_serve_file(ax_http_req_t* req, ax_http_res_t* res, ax_str file)
{
    return 0;
}

int ax_http_write_response(ax_http_hdlr_t* hdlr, ax_http_req_t* req, ax_http_res_t* res, ax_buf_t const* buf)
{
    return 0;
}

int ax_http_srv_stop(ax_http_srv_t* srv)
{
    return 0;
}

void ax_http_srv_destroy(ax_http_srv_t* srv)
{
}
