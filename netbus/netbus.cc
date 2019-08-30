#include <string.h>
#include <stdio.h>

#include <iostream>
#include <string>
    
using namespace std;

#include "uv.h"
#include "netbus.h"

extern "C"{

    static void after_write(uv_write_t* req, int status){
        if(status == 0){
            printf("write successed\n");
        }
        uv_buf_t* w_buf = (uv_buf_t*)req->data;
        if(w_buf){
            free(w_buf);
        }

        free(req);
    }

    static void on_close(uv_handle_t* handle){
        if(handle->data){
            free(handle->data);
            handle->data = NULL;
        }
    }

    static void on_shutdown(uv_shutdown_t* req, int status){
        uv_close((uv_handle_t*)req->handle, on_close);
        free(req);
    }

    static void after_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf){
        if(nread < 0){
            uv_shutdown_t* reg = (uv_shutdown_t*)malloc(sizeof(uv_shutdown_t));
            memset(reg,0,sizeof(uv_shutdown_t));
            uv_shutdown(reg, stream, on_shutdown);
            return;
        }

        buf->base[nread]=0;
        uv_write_t* w_req = (uv_write_t*)malloc(sizeof(uv_write_t));
        uv_buf_t* w_buf = (uv_buf_t*)malloc(sizeof(uv_buf_t));
        w_buf->base = buf->base;
        w_buf->len = nread;
        uv_write(w_req, stream, w_buf, 1, after_write);
    }

    static void uv_alloc_buf(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf){
        if(handle->data != NULL){
            free(handle->data);
            handle->data = NULL;
        }

        buf->base = (char*)malloc(suggested_size+1);
        buf->len = suggested_size;
        handle->data = buf->base;
    }

    static void uv_connection(uv_stream_t* server, int status){
        uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
        memset(client, 0, sizeof(uv_tcp_t));

        uv_tcp_init(uv_default_loop(), client);
        uv_accept(server, (uv_stream_t*)client);

        uv_read_start((uv_stream_t*)client,uv_alloc_buf, after_read);
    }

}

static netbus g_netbus;
netbus* netbus::instance(){
    return &g_netbus;
}

void netbus::start_tcp_server(int port){
    uv_tcp_t* listen = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    memset(listen, 0, sizeof(uv_tcp_t));

    uv_tcp_init(uv_default_loop(), listen);

    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", 6080, &addr);
    int ret = uv_tcp_bind(listen, (const struct sockaddr*) &addr, 0);
    if (ret != 0){
        printf("bind error \n");
        free(listen);
        return;
    }

    uv_listen((uv_stream_t*)listen, 128, uv_connection);
}


void netbus::run(){
    uv_run(uv_default_loop(),UV_RUN_DEFAULT);
}