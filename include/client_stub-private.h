#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "zookeeper/zookeeper.h"
#include "client_stub.h"
/*
    GRUPO 33
    - Ricardo Soares fc54446
    - Miguel Reis fc54409
    - Tiago Pinto fc55172
*/

typedef struct String_vector zoo_string; 
struct rtree_t {
    char* address_port;
    int socket_desc;
};

void* update_head_tail(zoo_string* children_list);

void my_watcher_func(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx);

static void child_watcher_cli(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx);

void* connectZookeeper(const char *address_port);

void zookeeper_client_close();



#endif
