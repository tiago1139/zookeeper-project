#include "tree_skel.h"
#include "sdmessage.pb-c.h"
#include "tree.h"
#include "tree-private.h"
#include "data.h"
#include "entry.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "tree-skel-private.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "zookeeper/zookeeper.h" // ZooKeeper

/*
    GRUPO 33
    - Ricardo Soares fc54446
    - Miguel Reis fc54409
    - Tiago Pinto fc55172
*/

/** * * * * * * * * * * * * * * *
*                               *
*       VARIAVEIS GLOBAIS       *
*                               *
*                               *
* * * * * * * * * * * * * * * * */

extern int sair; //variavel de saida quando o servidor eh fechado
extern char* ip_port; //IP e porta obtida em tree_server.c

char* child_path;
int child_id;
int has_next_server = 0; // next server == NULL
char* next_server_ip;
int next_server_id = 0;

int last_assigned;
int *thread_ids;
int n_threads;


struct rtree_t* remote_tree;
struct op_proc *op_proc;
struct request_t *queue_head; // subject to change -- fila de pedidos
struct tree_t *tree;

pthread_t *secondary_threads;

pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tree_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t max_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;


static zhandle_t *zh; //ZooKeeper Handler
typedef struct String_vector zoo_string; 

static char *watcher_ctx = "ZooKeeper Data Watcher";

/** * * * * * * * * * * * * * * *
*                               *
*       FUNCOES / METODOS       *
*                               *
*                               *
* * * * * * * * * * * * * * * * */

void zookeeper_serv_connect(const char* host_port){
    struct Stat stat;

    /* Connect to ZooKeeper server */
	zh = zookeeper_init(host_port, tree_skel_watcher,	10000, 0, NULL, 0); 
	if (zh == NULL)	{
		fprintf(stderr, "Error connecting to ZooKeeper server!\n");
	    exit(EXIT_FAILURE);
	}

    if(zoo_exists(zh, "/chain", 1, &stat) != ZOK) {
        int chain_path_len = 1024;
		char* chain_path = malloc (chain_path_len);
        if(zoo_create(zh, "/chain", NULL, -1, &ZOO_OPEN_ACL_UNSAFE, 0, chain_path, chain_path_len) != ZOK) {
            fprintf(stderr, "Error creating node Chain!\n");
	        exit(EXIT_FAILURE);
        }
        printf("**** Node Chain created! ****\n");
        printf("Node Chain Path: %s\n", chain_path);
        free(chain_path);
    }
    int child_path_len = 1024;
	child_path = malloc (child_path_len);
    if(zoo_create(zh, "/chain/node", ip_port, strlen(ip_port)+1, &ZOO_OPEN_ACL_UNSAFE, ZOO_SEQUENCE|ZOO_EPHEMERAL, child_path, child_path_len) != ZOK) {
        fprintf(stderr, "Error creating node Child!\n");
	    exit(EXIT_FAILURE);
    }
    
    printf("**** Node Child Ephemeral Sequential created. ****\n");
    printf("Node Child Path: %s\n", child_path);

    char *ptr = strtok(child_path, "/chain/node");
    printf("ID : %s\n", ptr);
    child_id = atoi(ptr);
    printf("CHILD ID : %d\n", child_id);

    zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));

    /* Get the list of children synchronously */
	int retval = zoo_get_children(zh, "/chain", 0, children_list); 
	if (retval != ZOK)	{
		fprintf(stderr, "Error retrieving znode from path %s!\n", "/chain");
	    exit(EXIT_FAILURE);
	}
	fprintf(stderr, "\n=== znode listing === [ %s ]", "/chain"); 
	for (int i = 0; i < children_list->count; i++)  {
		fprintf(stderr, "\n(%d): %s", i+1, children_list->data[i]); 
		free(children_list->data[i]);
	}
	fprintf(stderr, "\n=== done ===\n");

    if (ZOK != zoo_wget_children(zh, "/chain", &child_watcher, watcher_ctx, children_list)) {
				fprintf(stderr, "Error setting watch at %s!\n", "/chain");
	}
}

/* An empty Watcher function */
void tree_skel_watcher(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx) {}

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
	zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
	
	if (state == ZOO_CONNECTED_STATE)	 {
		if (type == ZOO_CHILD_EVENT) {
            printf("watching... \n");
            if(sair != 1){
        
	 	   /* Get the updated children and reset the watch */
 			if (ZOK != zoo_wget_children(zh, "/chain", child_watcher, watcher_ctx, children_list)) {
 				fprintf(stderr, "Error setting watch at %s!\n", "/chain");
 			}
            
			fprintf(stderr, "\n=== znode listing === [ %s ] \n", "/chain");
            char *znode_path = malloc(strlen(child_path));

            printf("child path mudado: %s\n", strncpy(znode_path, child_path + 7, strlen(child_path)));

            int max_temp = 0;
            int max = 0;
            int max_index = 0;
			for (int i = 0; i < children_list->count; i++)  {
                char *ptr = strtok(children_list->data[i], "/chain/node");
                int next_id = atoi(ptr);

                if(next_id > child_id) {
                    max_temp = next_id;
                    if(max == 0) {
                        max = next_id;
                        max_index = i;
                    }
                    if(max>max_temp) {
                        max = max_temp;
                        max_index = i;
                    }
                }
                          
                fprintf(stderr, "\n(%d): %s \n", i+1, children_list->data[i]);
				
			}

            if(max == 0) {
                
                printf("Nao existe next server! \n");
                has_next_server = 0;
            } else {
                if(next_server_id != max) {
                    next_server_id = max;
                    char* tmp_path = children_list->data[max_index];
                    char next_server_path[100] = "/chain/";
                    strcat(next_server_path, tmp_path);
                    int len = 1024;
                    if (zoo_get(zh, next_server_path, 0, next_server_ip, &len, NULL) == ZOK) {
                        printf("Wait for Connection please ....\n");
                        sleep(2);
                        remote_tree = rtree_connect(next_server_ip);
                        has_next_server = 1;
                        
                        if(remote_tree == NULL){
                            printf("next_server_id %d",next_server_id);
                        }
                        if(remote_tree !=NULL){
                            printf("Connection Established to next_server! \n");
                            printf("Next_server info : Socket : %d, Address: %s \n",remote_tree->socket_desc,remote_tree->address_port);
                        }
                        
                    } else {
                        printf("Ocorreu um erro. \n");
                    }
                } else {
                    printf("Next server eh o mesmo\n");

            }
                }
            }
			fprintf(stderr, "\n=== done ===\n");
		} 
	}
	free(children_list);
}

/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke().
 * A função deve lançar N threads secundárias responsáveis por atender
 * pedidos de escrita na árvore.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int tree_skel_init(int N)
{   
    
    tree = tree_create();
    if (tree == NULL)
    {
        return -1;
    }
    next_server_ip = malloc(1024);
    
    last_assigned = 1;
    int size = sizeof(int);
    op_proc = malloc(sizeof(struct op_proc));
    op_proc->in_progress = memset(malloc(size), 0, size); 
    op_proc->max_proc = 0;
    queue_head = NULL;
    secondary_threads = malloc(sizeof(pthread_t));
    thread_ids = malloc(sizeof(int));
    n_threads = 1;

    // Servidor passa apenas a ter uma Thread Secundaria
    // Conforme o Enunciado
    for (int i = 0; i < n_threads; i++)
    {
        // criação de threads
        thread_ids[i] = i;
        if (pthread_create(&secondary_threads[i], NULL, &process_request, (void *)&thread_ids[i]) != 0)
        { 
            printf("Thread %d não foi criada, \n", i);
            exit(1);
        }
    }

    printf("PASSOU TREE SKEL INIT\n");
    return 0;
}

void zookeeper_serv_disc(){
    free(child_path);
    zookeeper_close(zh);
}

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy()
{
    // Acordar as threads
    for(int i = 0; i< n_threads;i++){
        pthread_cond_broadcast(&queue_not_empty);
    }
    for(int i = 0;i < n_threads;i++){
        pthread_join(secondary_threads[i],NULL);
    }
    tree_destroy(tree);
    free(op_proc->in_progress);
    free(op_proc); 
    free(secondary_threads);
    free(thread_ids);
    zookeeper_serv_disc();

   
}

/* Verifica se a operação identificada por op_n foi executada.
 */
int verify(int op_n)
{   
    pthread_mutex_lock(&max_lock);
    if((op_proc->max_proc < op_n) || (op_n <= 0)){
        pthread_mutex_unlock(&max_lock);
        return -1; // valor não válido

    for(int i = 0;i < n_threads;i++){
        if(op_proc->in_progress[i] == op_n){ // operation in progress right now
            return 0;
        }
    }
    
    }if(op_proc->max_proc >=op_n){
        pthread_mutex_unlock(&max_lock);
        return 1;
    } else{
        pthread_mutex_unlock(&max_lock);
        return 0;
    }
}

/* Função da thread secundária que vai processar pedidos de escrita.
 * params pode ser NULL
 */
void *process_request(void *params)
{
    int *thread_id =(int *) params;
    while (!sair)
    {
        pthread_mutex_lock(&queue_lock);
        while (queue_head == NULL && !sair)
        {
            pthread_cond_wait(&queue_not_empty, &queue_lock); /* Espera haver algo */
        }
        if(!sair){
        struct request_t *request = queue_head;
        queue_head = request->next_request;
        pthread_mutex_unlock(&queue_lock);
        execute_request(request,*thread_id);

        free(request);
        
        }
        
    }
    pthread_exit(NULL);
}

void execute_request(struct request_t *request, int thread_id) 
{ 
    op_proc->in_progress[thread_id] = request->op_n;
    if (request->op == 0)
    {
        // DO DELETE
        printf("Thread %d a realizar operação delete \n",thread_id);
        pthread_mutex_lock(&tree_lock);
        int x = tree_del(tree, request->key);
        printf("Operação tree_del realizada. \n");
        if (x == -1)
        {
            printf("Chave %s não encontrada. \n", request->key);
            pthread_mutex_unlock(&tree_lock);
            free(request->key);
            
        }else {
            printf("Chave %s foi removida da árvore. \n", request->key);
            
            pthread_mutex_unlock(&tree_lock);
            pthread_mutex_lock(&max_lock);
            if(request->op_n > op_proc->max_proc){
                op_proc->max_proc = request->op_n;
                printf("new max_proc: %d \n",op_proc->max_proc);
            }
            pthread_mutex_unlock(&max_lock);
            
        }
        op_proc->in_progress[thread_id] = -1;
        if(has_next_server){
            if(rtree_del(remote_tree,request->key)==-1){
                printf("Erro na propagação do pedido de escrita del\n");
            }
        }

        free(request->key);
        //enviar mensagem para next_server
    }
    else if (request->op == 1)
    {
        // DO PUT
        //printf("Socket : %d, Address: %s, put \n",remote_tree->socket_desc,remote_tree->address_port);
        printf("Thread %d a realizar operação put \n",thread_id);
        pthread_mutex_lock(&tree_lock);
        int x = tree_put(tree, request->key, request->data);

        if (x == -1)
        {
            printf("Operação tree_put falhou. \n");
            pthread_mutex_unlock(&tree_lock);
            //return -1;
        }else {
            printf("Inserido na árvore através de tree_put os valores na mensagem \n");
            pthread_mutex_unlock(&tree_lock);
            pthread_mutex_lock(&max_lock);
            if(request->op_n > op_proc->max_proc){
                op_proc->max_proc = request->op_n;
                printf("new max_proc: %d \n",op_proc->max_proc);
            }
            pthread_mutex_unlock(&max_lock);
        }
        
        
        op_proc->in_progress[thread_id] = -1;
        if(has_next_server){
            printf("Nao \n");
            
            struct entry_t *entry = entry_create(request->key,request->data);
            if(rtree_put(remote_tree,entry)==-1){
                printf("Erro na propagação do pedido de escrita put \n");
            }
        }
        

        data_destroy(request->data);
        free(request->key);
    }
}

void queue_add_request(struct request_t *request)
{
    pthread_mutex_lock(&queue_lock);

    if (queue_head == NULL)
    { /* Adiciona na cabeça da fila */
        printf("queue_head empty \n");
        queue_head = request;
        request->next_request = NULL;
    }
    else
    { /* Adiciona no fim da fila */
        struct request_t *rptr = queue_head;
        while (rptr->next_request != NULL)
        {
            rptr = rptr->next_request;
        }
        rptr->next_request = request;
        request->next_request = NULL;
    }
    pthread_cond_signal(&queue_not_empty); /* Avisa um bloqueado nessa condição */
    pthread_mutex_unlock(&queue_lock);
}

/* Executa uma operação na árvore (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, árvore nao incializada)
 */
int invoke(MessageT *msg)
{

    switch (msg->opcode)
    {

    case MESSAGE_T__OPCODE__OP_SIZE:
    {
        
        pthread_mutex_lock(&tree_lock);
        int size = tree_size(tree);
        pthread_mutex_unlock(&tree_lock);
        printf("Operação size realizada, a devolver: %d \n", size);
        msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->value = size;
        return 0;
        break;
    }

    case MESSAGE_T__OPCODE__OP_HEIGHT:
    {
        pthread_mutex_lock(&tree_lock);
        int height = tree_height(tree);
        pthread_mutex_unlock(&tree_lock);
        printf("Operação height realizada, a devolver: %d \n", height);
        msg->opcode = MESSAGE_T__OPCODE__OP_HEIGHT + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->value = height;
        return 0;
        break;
    }

    case MESSAGE_T__OPCODE__OP_DEL:
    {
        
        struct request_t *request = malloc(sizeof(struct request_t));
        if(request == NULL){
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }
        request->key = malloc(strlen(msg->entry->key) + 1);
        if(request->key == NULL){
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }
        strcpy(request->key, msg->entry->key);
        request->op = 0;
        request->op_n = last_assigned; //
        request->data = NULL;
        last_assigned++;
        queue_add_request(request);

        msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        msg->operation_value = request->op_n;

        return 0;
        break;
    }

    case MESSAGE_T__OPCODE__OP_GET:
    {
        char *key = malloc(strlen(msg->entry->key) + 1);
        strcpy(key, msg->entry->key);
        pthread_mutex_lock(&tree_lock);
        struct data_t *data = tree_get(tree, key);
        pthread_mutex_unlock(&tree_lock);
        printf("Operação tree_get realizada. \n");
        if (data == NULL)
        {
            printf("Chave %s não encontrada / Ocorreu um erro a realizar a operação \n", key);
            free(key);
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->value = -1;
            msg->entry->data->datasize = 0;
            msg->entry->data->data = NULL;
            return -1;
        }
        printf("Valor associado à chave %s encontrado. \n", key);
        msg->opcode = (MESSAGE_T__OPCODE__OP_GET + 1);
        msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
        msg->entry->data->datasize = data->datasize;
        msg->entry->data->data = data->data;
        free(key);
        free(data);
        return 0;
        break;
    }

    case MESSAGE_T__OPCODE__OP_PUT:
    {
        struct request_t *request = malloc(sizeof(struct request_t));
        if(request == NULL){
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }
        request->key = malloc(strlen(msg->entry->key) + 1);
        if(request->key == NULL){
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }
        strcpy(request->key, msg->entry->key);
        void *data = malloc(msg->entry->data->datasize);
        if(data == NULL){
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }
        memcpy(data, msg->entry->data->data, msg->entry->data->datasize);
        request->data = data_create2(strlen(data)+1, data);
        if(request->data == NULL){
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }
        request->op_n = last_assigned;
        request->op = 1;
        last_assigned++;
        printf("adding request \n");
        queue_add_request(request);
        printf("request added \n");

        msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        msg->operation_value = request->op_n;

        return 0;
        break;
    }

    case MESSAGE_T__OPCODE__OP_GETKEYS:
    {
        pthread_mutex_lock(&tree_lock);
        char **keys = tree_get_keys(tree);
        pthread_mutex_unlock(&tree_lock);
        if (keys == NULL)
        {
            printf("Não há chaves na árvore \n");
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->n_keys = 0;
            msg->keys = NULL;
            return -1;
        }
        msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
        msg->n_keys = tree_size(tree);
        msg->keys = malloc(msg->n_keys * sizeof(char *));
        printf("Chaves da àrvore: \n");
        for (int i = 0; i < msg->n_keys; i++)
        {
            msg->keys[i] = strdup(keys[i]);
            printf("--------> %s \n", msg->keys[i]);
        }
        for (size_t i = 0;i < msg->n_keys;i++){
            free(keys[i]);
        }
        free(keys);

        return 0;
        break;
    }

    case MESSAGE_T__OPCODE__OP_GETVALUES:
    {
        pthread_mutex_lock(&tree_lock);
        void **values = tree_get_values(tree);
        pthread_mutex_unlock(&tree_lock);
        if (values == NULL)
        {
            printf("Não há valores na àrvore .\n");
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->n_values = 0;
            msg->values = NULL;
            return -1;
        }

        msg->opcode = MESSAGE_T__OPCODE__OP_GETVALUES + 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_VALUES;
        msg->n_values = tree_size(tree);
        msg->values = malloc(msg->n_values * sizeof(MessageT__DataT *));
        printf("A devolver lista de valores na arvore. \n");
        for (int i = 0; i < msg->n_values; i++)
        {
            struct data_t *temp = data_dup(values[i]);
            MessageT__DataT *dt = malloc(sizeof(MessageT__DataT));
            message_t__data_t__init(dt);
            dt->datasize = temp->datasize;
            dt->data = temp->data;

            memcpy(dt->data, temp->data, dt->datasize);
            msg->values[i] = dt;
            free(temp);
            
        }
        tree_free_values(values);
        return 0;
        break;
    }
        case MESSAGE_T__OPCODE__OP_VERIFY: { 
        int v = verify(msg->operation_value);
        if(v == -1){
            msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
            
        } else {
            msg->opcode = (MESSAGE_T__OPCODE__OP_VERIFY + 1);
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->value = v;
            return 0;
        }
        
        
        break;
        }
    }
}
