#ifndef _TREE_SKEL_PRIVATE_H
#define _TREE_SKEL_PRIVATE_H

#include "tree_skel.h"
#include "zookeeper/zookeeper.h"
/*
    GRUPO 33
    - Ricardo Soares fc54446
    - Miguel Reis fc54409
    - Tiago Pinto fc55172
*/

struct op_proc {
    int max_proc;
    int* in_progress;
};

struct request_t {
    int op_n; //o número da operação
    int op; //a operação a executar. op=0 se for um delete, op=1 se for um put
    char* key; //a chave a remover ou adicionar
    struct data_t *data; // os dados a adicionar em caso de put, ou NULL em caso de delete
    //MessageT *msg;
    struct request_t *next_request;
    //adicionar campo(s) necessário(s) para implementar fila do tipo produtor/consumidor
};


/*Adds a request to the queue_head of requests
*/
void queue_add_request(struct request_t *request);

/*Thread identified by thread_id executes the given request
*/
void execute_request(struct request_t *request, int thread_id);

void zookeeper_serv_connect(const char* host_port);


void tree_skel_watcher(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx);

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx);

void zookeeper_serv_disc();



#endif