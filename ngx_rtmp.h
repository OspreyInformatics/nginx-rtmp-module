/*
 * Copyright (c) 2012 Roman Arutyunyan
 */


#ifndef _NGX_RTMP_H_INCLUDED_
#define _NGX_RTMP_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_connect.h>


#define NGX_RTMP_HANDSHAKE_SIZE    1536

#define NGX_RTMP_DEFAULT_CHUNK_SIZE 128

#define NGX_LOG_DEBUG_RTMP NGX_LOG_DEBUG_CORE


typedef struct {
    void                  **main_conf;
    void                  **srv_conf;
} ngx_rtmp_conf_ctx_t;


typedef struct {
    u_char                  sockaddr[NGX_SOCKADDRLEN];
    socklen_t               socklen;

    /* server ctx */
    ngx_rtmp_conf_ctx_t    *ctx;

    unsigned                bind:1;
    unsigned                wildcard:1;
#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
    unsigned                ipv6only:2;
#endif
    unsigned                so_keepalive:2;
#if (NGX_HAVE_KEEPALIVE_TUNABLE)
    int                     tcp_keepidle;
    int                     tcp_keepintvl;
    int                     tcp_keepcnt;
#endif
} ngx_rtmp_listen_t;


typedef struct {
    ngx_rtmp_conf_ctx_t    *ctx;
    ngx_str_t               addr_text;
} ngx_rtmp_addr_conf_t;

typedef struct {
    in_addr_t               addr;
    ngx_rtmp_addr_conf_t    conf;
} ngx_rtmp_in_addr_t;


#if (NGX_HAVE_INET6)

typedef struct {
    struct in6_addr         addr6;
    ngx_rtmp_addr_conf_t    conf;
} ngx_rtmp_in6_addr_t;

#endif


typedef struct {
    void                   *addrs;
    ngx_uint_t              naddrs;
} ngx_rtmp_port_t;


typedef struct {
    int                     family;
    in_port_t               port;
    ngx_array_t             addrs;       /* array of ngx_rtmp_conf_addr_t */
} ngx_rtmp_conf_port_t;


typedef struct {
    struct sockaddr        *sockaddr;
    socklen_t               socklen;

    ngx_rtmp_conf_ctx_t    *ctx;

    unsigned                bind:1;
    unsigned                wildcard:1;
#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
    unsigned                ipv6only:2;
#endif
    unsigned                so_keepalive:2;
#if (NGX_HAVE_KEEPALIVE_TUNABLE)
    int                     tcp_keepidle;
    int                     tcp_keepintvl;
    int                     tcp_keepcnt;
#endif
} ngx_rtmp_conf_addr_t;


typedef struct {
    ngx_array_t             servers;    /* ngx_rtmp_core_srv_conf_t */
    ngx_array_t             listen;     /* ngx_rtmp_listen_t */
} ngx_rtmp_core_main_conf_t;


typedef struct {
    uint32_t                csid;       /* chunk stream id */
    uint32_t                timestamp;
    uint32_t                mlen;       /* message length */
    uint8_t                 type;       /* message type id */
    uint32_t                msid;       /* message stream id */
} ngx_rtmp_packet_hdr_t;


#define NGX_RTMP_PUBLISHER   0x01
#define NGX_RTMP_SUBSCRIBER  0x02


typedef struct ngx_rtmp_stream_t {
    ngx_rtmp_packet_hdr_t   hdr;
    ngx_chain_t            *in;
} ngx_rtmp_stream_t;


struct ngx_rtmp_session_s {
    uint32_t                signature;         /* "RTMP" */

    ngx_connection_t       *connection;

    void                  **ctx;
    void                  **main_conf;
    void                  **srv_conf;

    ngx_str_t              *addr_text;

    /* handshake */
    ngx_buf_t               buf;
    ngx_uint_t              hs_stage;

    /* input
     * stream 0 (reserved by RTMP spec)
     * used for free chain link */

    /* TODO: make stream #1 handle ANY single stream;
     * that'll introduce support for
     * unlimited number of streams given
     * there's no interleaving between them */

    ngx_rtmp_stream_t      *streams;
    uint32_t                in_csid;
    ngx_uint_t              in_chunk_size;
    ngx_pool_t             *in_pool;

    /* output */
    ngx_chain_t            *out;
    ngx_rtmp_packet_hdr_t   out_hdr;

    /* broadcast */
    ngx_str_t               name;
    struct ngx_rtmp_session_s
                            *next;
    ngx_uint_t              flags;
    uint32_t                csid;
};

typedef struct ngx_rtmp_session_s ngx_rtmp_session_t;


#define NGX_RTMP_SESSION_HASH_SIZE 16384


typedef struct {
    ngx_msec_t              timeout;
    ngx_flag_t              so_keepalive;
    ngx_int_t               max_streams;
    
    /* shared output buffers */
    ngx_uint_t              out_chunk_size;
    ngx_pool_t             *pool;
    ngx_chain_t            *free;

    ngx_rtmp_session_t    **sessions;  /* session hash map: name->session */

    ngx_rtmp_conf_ctx_t    *ctx;
} ngx_rtmp_core_srv_conf_t;


typedef struct {
    ngx_str_t              *client;
    ngx_rtmp_session_t     *session;
} ngx_rtmp_log_ctx_t;


typedef struct {
    void                 *(*create_main_conf)(ngx_conf_t *cf);
    char                 *(*init_main_conf)(ngx_conf_t *cf, void *conf);

    void                 *(*create_srv_conf)(ngx_conf_t *cf);
    char                 *(*merge_srv_conf)(ngx_conf_t *cf, void *prev,
                                      void *conf);
} ngx_rtmp_module_t;


/* Chunk header:
 *   max 3  basic header
 * + max 11 message header
 * + max 4  extended header (timestamp) */
#define NGX_RTMP_MAX_CHUNK_HEADER       18


/* RTMP message types*/
#define NGX_RTMP_MSG_CHUNK_SIZE         1
#define NGX_RTMP_MSG_ABORT              2
#define NGX_RTMP_MSG_ACK                3
#define NGX_RTMP_MSG_USER               4
#define NGX_RTMP_MSG_ACK_SIZE           5
#define NGX_RTMP_MSG_BANDWIDTH          6
#define NGX_RTMP_MSG_EDGE               7
#define NGX_RTMP_MSG_AUDIO              8
#define NGX_RTMP_MSG_VIDEO              9
#define NGX_RTMP_MSG_AMF3_META          15
#define NGX_RTMP_MSG_AMF3_SHARED        16
#define NGX_RTMP_MSG_AMF3_CMD           17
#define NGX_RTMP_MSG_AMF0_META          18
#define NGX_RTMP_MSG_AMF0_SHARED        19
#define NGX_RTMP_MSG_AMF0_CMD           20
#define NGX_RTMP_MSG_AGGREGATE          22


/* RMTP control message types */
#define NGX_RTMP_USER_STREAM_BEGIN      0
#define NGX_RTMP_USER_STREAM_EOF        1
#define NGX_RTMP_USER_STREAM_DRY        2
#define NGX_RTMP_USER_SET_BUFLEN        3
#define NGX_RTMP_USER_RECORDED          4
#define NGX_RTMP_USER_PING_REQUEST      6
#define NGX_RTMP_USER_PING_RESPONSE     7


#define NGX_RTMP_MODULE                 0x504D5452     /* "RTMP" */

#define NGX_RTMP_MAIN_CONF              0x02000000
#define NGX_RTMP_SRV_CONF               0x04000000


#define NGX_RTMP_MAIN_CONF_OFFSET  offsetof(ngx_rtmp_conf_ctx_t, main_conf)
#define NGX_RTMP_SRV_CONF_OFFSET   offsetof(ngx_rtmp_conf_ctx_t, srv_conf)


#define ngx_rtmp_get_module_ctx(s, module)     (s)->ctx[module.ctx_index]
#define ngx_rtmp_set_ctx(s, c, module)         s->ctx[module.ctx_index] = c;
#define ngx_rtmp_delete_ctx(s, module)         s->ctx[module.ctx_index] = NULL;


#define ngx_rtmp_get_module_main_conf(s, module)                             \
    (s)->main_conf[module.ctx_index]
#define ngx_rtmp_get_module_srv_conf(s, module)  (s)->srv_conf[module.ctx_index]

#define ngx_rtmp_conf_get_module_main_conf(cf, module)                       \
    ((ngx_rtmp_conf_ctx_t *) cf->ctx)->main_conf[module.ctx_index]
#define ngx_rtmp_conf_get_module_srv_conf(cf, module)                        \
    ((ngx_rtmp_conf_ctx_t *) cf->ctx)->srv_conf[module.ctx_index]

void ngx_rtmp_init_connection(ngx_connection_t *c);    
void ngx_rtmp_close_session(ngx_rtmp_session_t *s);
u_char * ngx_rtmp_log_error(ngx_log_t *log, u_char *buf, size_t len);

/* Sending messages */
ngx_chain_t * ngx_rtmp_alloc_shared_buf(ngx_rtmp_session_t *s);
void ngx_rtmp_prepare_message(ngx_rtmp_packet_hdr_t *h, 
        ngx_chain_t *out, uint8_t fmt);
void ngx_rtmp_send_message(ngx_rtmp_session_t *s, ngx_chain_t *out);

#define NGX_RTMP_LIMIT_SOFT         0
#define NGX_RTMP_LIMIT_HARD         1
#define NGX_RTMP_LIMIT_DYNAMIC      2

/* Protocol control messages */
ngx_int_t ngx_rtmp_send_chunk_size(ngx_rtmp_session_t *s, 
        uint32_t chunk_size);
ngx_int_t ngx_rtmp_send_abort(ngx_rtmp_session_t *s, 
        uint32_t csid);
ngx_int_t ngx_rtmp_send_ack(ngx_rtmp_session_t *s, 
        uint32_t seq);
ngx_int_t ngx_rtmp_send_ack_size(ngx_rtmp_session_t *s, 
        uint32_t ack_size);
ngx_int_t ngx_rtmp_send_bandwidth(ngx_rtmp_session_r *s, 
        uint32_t ack_size, uint8_t limit_type);

/* User control messages */
ngx_int_t ngx_rtmp_send_user_stream_begin(ngx_rtmp_session_t *s, 
        uint32_t msid);
ngx_int_t ngx_rtmp_send_user_stream_eof(ngx_rtmp_session_t *s, 
        uint32_t msid);
ngx_int_t ngx_rtmp_send_user_stream_dry(ngx_rtmp_session_t *s, 
        uint32_t msid);
ngx_int_t ngx_rtmp_send_user_set_buflen(ngx_rtmp_session_t *s, 
        uint32_t msid, uint32_t buflen_msec);
ngx_int_t ngx_rtmp_send_user_recorded(ngx_rtmp_session_t *s, 
        uint32_t msid);
ngx_int_t ngx_rtmp_send_user_ping_request(ngx_rtmp_session_t *s,
        uint32_t timestamp);
ngx_int_t ngx_rtmp_send_user_ping_response(ngx_rtmp_session_t *s, 
        uint32_t timestamp);

/* AMF0 sender/receiver */
ngx_int_t ngx_rtmp_send_amf0(ngx_session_t *s, 
        uint32_t csid, uint32_t msid,
        ngx_rtmp_amf0_elt_t *elts, size_t nelts);
ngx_int_t ngx_rtmp_receive_amf0(ngx_session_t *s, ngx_chain_t *in, 
        ngx_rtmp_amf0_elt_t *elts, size_t nelts)


/************** will go to modules */

/* Broadcasting */
void ngx_rtmp_join(ngx_rtmp_session_t *s, ngx_str_t *name, ngx_uint_t flags);
void ngx_rtmp_leave(ngx_rtmp_session_t *s);

/* NetConnection methods */
ngx_int_t ngx_rtmp_connect(ngx_rtmp_session_t *s, 
        double trans_id, ngx_chain_t *l);
ngx_int_t ngx_rtmp_call(ngx_rtmp_session_t *s,
        double trans_id, ngx_chain_t *l);
ngx_int_t ngx_rtmp_close(ngx_rtmp_session_t *s,
        double trans_id, ngx_chain_t *l);
ngx_int_t ngx_rtmp_createstream(ngx_rtmp_session_t *s
        double trans_id, , ngx_chain_t *l);

/* NetStream methods */
ngx_int_t ngx_rtmp_play(ngx_rtmp_session_t *s,
        double trans_id, ngx_chain_t *l);
ngx_int_t ngx_rtmp_play2(ngx_rtmp_session_t *s,
        double trans_id, ngx_chain_t *l);
ngx_int_t ngx_rtmp_deletestream(ngx_rtmp_session_t *s,
        double trans_id, ngx_chain_t *l);
ngx_int_t ngx_rtmp_closestream(ngx_rtmp_session_t *s,
        double trans_id, ngx_chain_t *l);
ngx_int_t ngx_rtmp_receiveaudio(ngx_rtmp_session_t *s,
        double trans_id, ngx_chain_t *l);
ngx_int_t ngx_rtmp_receivevideo(ngx_rtmp_session_t *s,
        double trans_id, ngx_chain_t *l);
ngx_int_t ngx_rtmp_publish(ngx_rtmp_session_t *s,
        double trans_id, ngx_chain_t *l);
ngx_int_t ngx_rtmp_seek(ngx_rtmp_session_t *s,
        double trans_id, ngx_chain_t *l);
ngx_int_t ngx_rtmp_pause(ngx_rtmp_session_t *s,
        double trans_id, ngx_chain_t *l);

extern ngx_uint_t    ngx_rtmp_max_module;
extern ngx_module_t  ngx_rtmp_core_module;


#endif /* _NGX_RTMP_H_INCLUDED_ */