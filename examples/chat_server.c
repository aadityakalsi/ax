/*
For license details see ../LICENSE
*/

/**
 * \file chat_server.c
 * \date Aug 28, 2017
 */

#define AX_MIN_LOG_LEVEL AX_LOG_INFO
#include "ax/tcp.h"
#include "ax/log.h"
#include "ax/assert.h"
#include "ax/pool.h"

#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define CHECK(x)   \
  {                \
    int ret = (x); \
    if (ret) {     \
        AX_LOG(FAIL, #x ": %s\n", ax_error_str(ret)); \
        return ret;\
    }              \
  }

typedef struct chat_user_s chat_user_t;
typedef struct chat_msg_s chat_msg_t;

#define USR_SIZE 128
#define MSG_SIZE 4096

struct chat_msg_s
{
    int ref;
    int size;
    char usr[USR_SIZE];
    char msg[MSG_SIZE];
};

struct chat_user_s
{
    char name[USR_SIZE];
    ax_sz id;
    char buf_rd[MSG_SIZE];
    char buf_wr[MSG_SIZE];
    chat_user_t* prev;
    chat_user_t* next;
    ax_tcp_req_t* req;
    int delete;
};

typedef struct
{
    chat_user_t user;
    ax_sz num;
    ax_sz next_id;
    ax_pool_t msgs;
} chat_srv_t;

static chat_srv_t chat;
static ax_tcp_srv_t chat_srv;

static
chat_msg_t* copy_msg(chat_msg_t* m)
{
    ++(m->ref);
    return m;
}

static
chat_msg_t* delete_msg(chat_msg_t* m)
{
    if (--(m->ref) == 0) {
        ax_pool_free(&chat.msgs, m);
    }
}

static
chat_msg_t* create_msg(ax_const_str usr, ax_buf_t const* b)
{
    chat_msg_t* m = (chat_msg_t*)ax_pool_alloc(&chat.msgs);
    usr = usr ? usr : "**";
    AX_ASSERT(m);
    strcpy(m->usr, usr);
    sprintf(m->msg, "%s: %.*s", usr, b->len, b->data);
    m->ref = 0;
    m->size = strlen(m->msg);
    return m;
}

static
void after_msg_post(void* req_ctx, ax_buf_t const* buf)
{
    chat_msg_t* m = (chat_msg_t*)((ax_u8*)buf->data - offsetof(chat_msg_t, msg));
    delete_msg(m);
}

static
void msg_post_status(void* req_ctx, int err)
{
}

static
void post_msg_to_user(chat_user_t* u, chat_msg_t* m)
{
    ax_buf_t buf = { m->msg, m->size };
    copy_msg(m);
    ax_tcp_srv_write(&chat_srv, u->req, &buf, after_msg_post, msg_post_status);
}

/* maybe use a &m->msg -> m map and clear entry when all refs are gone */
static
void post_msg_to_users(chat_msg_t* m)
{
    chat_user_t* end = &chat.user;
    chat_user_t* beg = end->next;
    while (beg != end) {
        if (strcmp(beg->name, m->usr) != 0) {
            post_msg_to_user(beg, m);
        }
        beg = beg->next;
    }
}

static
void req_get_read_buf(void* c, ax_buf_t* buf)
{
    chat_user_t* u = (chat_user_t*)c;
    if (u->delete) {
        buf->data = AX_STOP_READ_NO_WRITE;
    } else {
        buf->data = u->buf_rd;
        buf->len = sizeof(u->buf_rd);
    }
}

static
void req_free_read_buf(void* c, ax_buf_t const* buf)
{
}

static
void req_get_write_buf(void* c, ax_buf_t* buf)
{
    chat_user_t* u = (chat_user_t*)c;
    buf->data = u->buf_wr;
    buf->len = sizeof(u->buf_wr);
}

static
void req_free_write_buf(void* c, ax_buf_t const* buf)
{
}

static
void req_read_cbk(void* c, int err, ax_buf_t const* buf)
{
    /* post to all chat users */
    chat_user_t* u = (chat_user_t*)c;
    chat_msg_t* m;
    char buff[MSG_SIZE];
    ax_buf_t rbuf;
    if (buf->len == 6 && strncmp(buf->data, "quit\r\n", 6) == 0) {
        sprintf(buff, "%s left the building.\n", u->name);
        rbuf.data = buff;
        rbuf.len = (int)strlen(buff);
        u->delete = 1;
        m = create_msg(AX_NULL, &rbuf);
    } else {
        m = create_msg(u->name, buf);
    }
    post_msg_to_users(m);
}

static
void req_write_cbk(void* c, int err)
{
    /* finished writing to user */
}

static
void srv_init_req(void* p, ax_tcp_req_t* req)
{
    chat_srv_t* s = (chat_srv_t*)p;
    chat_user_t* u = (chat_user_t*)malloc(sizeof(chat_user_t));
    char buff[MSG_SIZE];
    char welcome[MSG_SIZE];
    ax_buf_t b;
    if (!u) return;
    u->prev = s->user.prev;
    s->user.prev->next = u;
    u->next = &s->user;
    s->user.prev = u;
    u->id = (s->next_id)++;
    u->req = req;
    u->delete = 0;
    ++(s->num);
    sprintf(u->name, "user%zu", u->id);
    req->req_ctx = u;
    req->get_read_buf = req_get_read_buf;
    req->free_read_buf = req_free_read_buf;
    req->get_write_buf = req_get_write_buf;
    req->free_write_buf = req_free_write_buf;
    req->read_cbk = req_read_cbk;
    req->write_cbk = req_write_cbk;
    sprintf(welcome, "\n"
                     "-- Welcome to the Chat Room --\n"
                     "-- your name: %13s --\n"
                     "-- Welcome to the Chat Room --\n", u->name);
    sprintf(buff, "%s joined the room\n", u->name);
    b.data = welcome;
    b.len = (ax_i32)strlen(welcome);
    post_msg_to_user(u, create_msg(AX_NULL, &b));
    b.data = buff;
    b.len = (ax_i32)strlen(buff);
    post_msg_to_users(create_msg(AX_NULL, &b));
}

static
void srv_destroy_req(void* p, ax_tcp_req_t* req)
{
    chat_srv_t* s = (chat_srv_t*)p;
    chat_user_t* u = req->req_ctx;
    u->prev->next = u->next;
    u->next->prev = u->prev;
    --(s->num);
    free(u);
}

static
void srv_on_start(void* s)
{
}

static
void sigint_hdler(int sig)
{
    ax_tcp_srv_destroy(&chat_srv);
    ax_pool_destroy(&chat.msgs);
    exit(sig);
}

int main(int argc, ax_const_str argv[])
{
    ax_const_str ip   = argc > 1 ? argv[1] : "localhost";
    ax_const_str port = argc > 2 ? argv[2] : "8080";
    ax_tcp_ctx_t ctx = {
        &chat,
        srv_on_start,
        srv_init_req,
        srv_destroy_req
    };

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, sigint_hdler);
    
    chat.user.prev = &chat.user;
    chat.user.next = &chat.user;
    chat.num = chat.next_id = 0;
    CHECK(ax_pool_init(&chat.msgs, sizeof(chat_msg_t)));

    CHECK(ax_tcp_srv_init_ip4(&chat_srv, ip, atoi(port)));
    ax_tcp_srv_set_ctx(&chat_srv, &ctx);
    ax_tcp_srv_start(&chat_srv);
    CHECK(ax_tcp_srv_destroy(&chat_srv));
    CHECK(ax_pool_destroy(&chat.msgs));
    return 0;
}
