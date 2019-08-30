#include <string.h>
#include <stdio.h>

#include <iostream>
#include <string>

using namespace std;

#include "uv.h"
#include "netbus.h"

static netbus g_netbus;
netbus* netbus::instance(){
    return &g_netbus;
}

void start_tcp_server(int port){
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
}
