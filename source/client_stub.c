#include "client_stub.h"
#include "sdmessage.pb-c.h"
#include "client_stub-private.h"
#include "network_client.h"

#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "inet.h"

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


/* Remote tree. A definir pelo grupo em client_stub-private.h
 */
struct rtree_t;
struct rtree_t* head;
struct rtree_t* tail;

char* head_server_ip;
char* tail_server_ip;

int head_server_id = -1;
int tail_server_id = -1;

extern int fechar;

static zhandle_t *zh; //ZooKeeper Handler
typedef struct String_vector zoo_string; 
static char *watcher_ctx = "ZooKeeper Data Watcher";

/** * * * * * * * * * * * * * * *
*                               *
*       FUNCOES / METODOS       *
*                               *
*                               *
* * * * * * * * * * * * * * * * */

/* Função para estabelecer uma associação entre o cliente e o servidor, 
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */

void* update_head_tail(zoo_string* children_list){
    int min_id = -1;
    int min_index = 0;
    int max_id = -1;
    int max_index = 0;

    fprintf(stderr, "\n=== znode listing === [ %s ]", "/chain");
    printf("count %d \n",children_list->count);
    if (children_list->count > 0){
        for (int i = 0; i < children_list->count; i++)  {
            char *ptr = strtok(children_list->data[i], "/chain/node");
            int next_id = atoi(ptr);
            printf("NEXT ID : %d\n", next_id);
            if(min_id == -1){
                min_id = next_id;
                min_index = i;
            } else {
                if(min_id > next_id){
                    min_id = next_id;
                    min_index = i;
                }
            }
            if(max_id == -1){
                max_id = next_id;
                max_index = i;
            } else {
                if(max_id < next_id){
                    max_id = next_id;
                    max_index = i;
                }
            }
        }

    // TAIL
    // A tail precisa sempre ser atualizada a conexao mesmo que seja a mesma
    // para evitar problemas de concorrencia na leitura dos servidores.
    if(tail_server_id == -1){
        tail_server_ip = malloc(1024);
    }
    else {
        free(tail_server_ip);
        tail_server_ip = malloc(1024);
    }
    tail_server_id = max_id;
    char* tmp_path = children_list->data[max_index];
    char next_server_path[100] = "/chain/";
    strcat(next_server_path, tmp_path);
    int len = 1024;
    if (zoo_get(zh, next_server_path, 0, tail_server_ip, &len, NULL) == ZOK) {
        printf("Wait for Connection please ....\n");
        sleep(3);
        tail = rtree_connect(tail_server_ip);
        if(tail != NULL){
            printf("Tail Address Port: %s , Socket: %d \n",tail->address_port,tail->socket_desc);
        }
        
    }
    //HEAD
    if(head_server_id == -1 || head_server_id != min_id){
        if(head_server_id == -1){
            head_server_ip = malloc(1024);
        }
        else {
            free(head_server_ip);
            head_server_ip = malloc(1024);
        }
        head_server_id = min_id;
        char* tmp_path = children_list->data[min_index];
        char next_server_path[100] = "/chain/";
        strcat(next_server_path, tmp_path);
        int len = 1024;
        if (zoo_get(zh, next_server_path, 0, head_server_ip, &len, NULL) == ZOK) {
            printf("Wait for Connection please ....\n");
            sleep(3);
            head = rtree_connect(head_server_ip);
            if(head != NULL){
                printf("Head Address: %s , Socket: %d \n",head->address_port,head->socket_desc);
            }
            
        }
    }else {
        printf("Head igual \n");
    }
    }
    

}

void my_watcher_func(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx) {}

static void child_watcher_cli(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
    printf("Entrou no watch\n");
	zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
	
	if (state == ZOO_CONNECTED_STATE)	 {
		if (type == ZOO_CHILD_EVENT) {
        
	 	   /* Get the updated children and reset the watch */
 			if (ZOK != zoo_wget_children(zh, "/chain", child_watcher_cli, watcher_ctx, children_list)) {
 				fprintf(stderr, "Error setting watch at %s!\n", "/chain");
 			}
            update_head_tail(children_list);
            
        }
    }
}
void* connectZookeeper(const char *address_port){
    zh = zookeeper_init(address_port, my_watcher_func,	10000, 0, NULL, 0); 
	if (zh == NULL)	{
		fprintf(stderr, "Error connecting to ZooKeeper server!\n");
	    exit(EXIT_FAILURE);
	}    
    zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
    int retval = zoo_get_children(zh, "/chain", 0, children_list); 
	if (retval != ZOK)	{
		fprintf(stderr, "Error retrieving znode from path %s!\n", "/chain");
	    exit(EXIT_FAILURE);
	}
    printf("Updating Head and Tail server for the first time! \n");
    update_head_tail(children_list);
    printf("Done! \n");
	fprintf(stderr, "\n=== znode listing === [ %s ]", "/chain"); 
	for (int i = 0; i < children_list->count; i++)  {
		fprintf(stderr, "\n(%d): %s", i+1, children_list->data[i]); 
		free(children_list->data[i]);
	}
	fprintf(stderr, "\n=== done ===\n");
    
    if (ZOK != zoo_wget_children(zh, "/chain", &child_watcher_cli, watcher_ctx, children_list)) {
				fprintf(stderr, "Error setting watch at %s!\n", "/chain");
	}
}



struct rtree_t *rtree_connect(const char *address_port){
    
    struct rtree_t *rtree = malloc(sizeof(struct rtree_t));
    rtree->address_port = malloc(strlen(address_port)+1);
    strcpy(rtree->address_port,address_port);
    int v = network_connect(rtree);
    if (v == -1){
        perror("nao conseguiu connectar\n");
        free(rtree->address_port);
        free(rtree);
        return NULL;
    }
    printf("remote tree established. \n");
    return rtree;
    
}

/* Termina a associação entre o cliente e o servidor, fechando a 
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtree_disconnect(struct rtree_t *rtree){
    if(network_close(rtree) == -1){ // se dá erro faz sentido libertar a memória?
        free(rtree->address_port);
        free(rtree);
        return -1;
    }
    free(rtree->address_port);
    free(rtree);
    return 0;
}
void zookeeper_client_close(){
    zookeeper_close(zh);
}
/* Função para adicionar um elemento na árvore.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Devolve 0 (ok, em adição/substituição) ou -1 (problemas).
 */
int rtree_put(struct rtree_t *rtree, struct entry_t *entry){
    MessageT mensagem;

    message_t__init(&mensagem);

    mensagem.entry = malloc(sizeof(MessageT__EntryT));
    if(mensagem.entry == NULL){
        return -1;
    }
    message_t__entry_t__init(mensagem.entry);

    mensagem.entry->data = malloc(sizeof(MessageT__DataT));
    if(mensagem.entry->data == NULL){
        return -1;
    }
    message_t__data_t__init(mensagem.entry->data);

    mensagem.opcode = MESSAGE_T__OPCODE__OP_PUT;
    mensagem.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
    mensagem.entry->key = entry->key;
    mensagem.entry->data->datasize = entry->value->datasize;
    mensagem.entry->data->data = entry->value->data;
    printf("hello \n");
    MessageT *resposta = network_send_receive(rtree,&mensagem);
     if(resposta == NULL || resposta->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        printf("Erro a inserir \n");
        free(mensagem.entry->data);
        free(mensagem.entry);
        message_t__free_unpacked(resposta, NULL);
        return -1;
    }
    if(resposta->opcode == (MESSAGE_T__OPCODE__OP_PUT + 1)){
        free(mensagem.entry->data);
        free(mensagem.entry);
        int v = resposta->operation_value;
        message_t__free_unpacked(resposta, NULL);
        return v;
    }
}

/* Função para obter um elemento da árvore.
 * Em caso de erro, devolve NULL.
 */
struct data_t *rtree_get(struct rtree_t *rtree, char *key){
    MessageT mensagem;

    message_t__init(&mensagem);

    mensagem.entry = malloc(sizeof(MessageT__EntryT));
    if(mensagem.entry == NULL){
        return NULL;
    }
    message_t__entry_t__init(mensagem.entry);

    mensagem.entry->data = malloc(sizeof(MessageT__DataT));
    if(mensagem.entry->data == NULL){
        return NULL;
    }
    message_t__data_t__init(mensagem.entry->data);

    mensagem.opcode = MESSAGE_T__OPCODE__OP_GET;
    mensagem.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    mensagem.entry->key = key;
    MessageT *resposta = network_send_receive(rtree,&mensagem);

    if(resposta->opcode == (MESSAGE_T__OPCODE__OP_GET + 1)){
        
        struct data_t *data = data_create(resposta->entry->data->datasize);
        memcpy(data->data, resposta->entry->data->data, data->datasize);
        free(mensagem.entry->data);
        free(mensagem.entry);
        message_t__free_unpacked(resposta, NULL);
        return data;
    }
    if(resposta == NULL || resposta->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        printf("Valor não existente na tree/ ocorreu um erro \n");
        free(mensagem.entry->data);
        free(mensagem.entry);
        message_t__free_unpacked(resposta, NULL);
        return NULL;
    }
    
}

/* Função para remover um elemento da árvore. Vai libertar 
 * toda a memoria alocada na respetiva operação rtree_put().
 * Devolve: 0 (ok), -1 (key not found ou problemas).
 */
int rtree_del(struct rtree_t *rtree, char *key){
    MessageT mensagem;

    message_t__init(&mensagem);

    mensagem.entry = malloc(sizeof(MessageT__EntryT));
    if(mensagem.entry == NULL){
        return -1;
    }
    message_t__entry_t__init(mensagem.entry);
    mensagem.opcode = MESSAGE_T__OPCODE__OP_DEL;
    mensagem.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    mensagem.entry->key = key;

    MessageT *resposta = network_send_receive(rtree,&mensagem);
    if(resposta == NULL || resposta->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        
        free(mensagem.entry);
        message_t__free_unpacked(resposta, NULL);
        return -1;
    }
    if(resposta->opcode == (MESSAGE_T__OPCODE__OP_DEL + 1)){
        free(mensagem.entry); 
        int v = resposta->operation_value;
        message_t__free_unpacked(resposta, NULL);
        return v;
    }
    

}

/* Devolve o número de elementos contidos na árvore.
 */
int rtree_size(struct rtree_t *rtree){
    MessageT mensagem;
    message_t__init(&mensagem);
    
    mensagem.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    mensagem.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *resposta = network_send_receive(rtree,&mensagem);
    if(resposta == NULL || resposta->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        message_t__free_unpacked(resposta, NULL);
        return -1;
    }
    if(resposta->opcode == (MESSAGE_T__OPCODE__OP_SIZE + 1)){
        int size = resposta->value;
   
        message_t__free_unpacked(resposta, NULL);
        return size;
    }
}

/* Função que devolve a altura da árvore.
 */
int rtree_height(struct rtree_t *rtree){
    MessageT mensagem;

    message_t__init(&mensagem);
    mensagem.opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
    mensagem.c_type = MESSAGE_T__C_TYPE__CT_NONE;
    MessageT *resposta = network_send_receive(rtree,&mensagem);
     if(resposta == NULL || resposta->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        message_t__free_unpacked(resposta, NULL);
        return -1;
    }
    if(resposta->opcode == (MESSAGE_T__OPCODE__OP_HEIGHT + 1)){
        int value = resposta->value;
        message_t__free_unpacked(resposta, NULL);
        return value;
    }

}

/* Devolve um array de char* com a cópia de todas as keys da árvore,
 * colocando um último elemento a NULL.
 */
char **rtree_get_keys(struct rtree_t *rtree){
    MessageT mensagem;

    message_t__init(&mensagem);
    mensagem.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    mensagem.c_type = MESSAGE_T__C_TYPE__CT_NONE;
    mensagem.n_keys = 0;
    mensagem.keys = NULL;
    MessageT *resposta = network_send_receive(rtree,&mensagem);
     if(resposta == NULL || resposta->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        message_t__free_unpacked(resposta, NULL);
        return NULL;
    }
    if(resposta->opcode == (MESSAGE_T__OPCODE__OP_GETKEYS + 1)){
        char **keys;
        int size = resposta->n_keys;// ultimo da string
        keys = calloc(size+1,sizeof(char*));
        int i;
        for(i = 0; i < size; i++){
            keys[i] = malloc(strlen(resposta->keys[i])+1);
            strcpy(keys[i], resposta->keys[i]);
            
        }
        keys[size] = NULL;
        message_t__free_unpacked(resposta, NULL);
        return keys;   
    }

}

/* Devolve um array de void* com a cópia de todas os values da árvore,
 * colocando um último elemento a NULL.
 */
void **rtree_get_values(struct rtree_t *rtree){
    MessageT mensagem;

    message_t__init(&mensagem);
    mensagem.opcode = MESSAGE_T__OPCODE__OP_GETVALUES;
    mensagem.c_type = MESSAGE_T__C_TYPE__CT_NONE;
    mensagem.n_values = 0;
    mensagem.values = NULL;
    MessageT *resposta = network_send_receive(rtree,&mensagem);
     if(resposta == NULL || resposta->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        message_t__free_unpacked(resposta, NULL);
        return NULL;
    }
    if(resposta->opcode == (MESSAGE_T__OPCODE__OP_GETVALUES + 1)){
        void **values;
        int size = resposta->n_values;
        values = calloc(size+1,sizeof(void*));
        int i;
        for(i=0;i<size;i++){
            values[i] = strdup(resposta->values[i]->data);
            
        }
        
        values[size] = NULL;
        message_t__free_unpacked(resposta, NULL);
        printf("\n###################################\n");
        return values;
    }
}

int rtree_verify(struct rtree_t *rtree, int op_n){
    MessageT mensagem;

    message_t__init(&mensagem);
    mensagem.opcode = MESSAGE_T__OPCODE__OP_VERIFY;
    mensagem.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
    mensagem.operation_value = op_n;

    MessageT *resposta = network_send_receive(rtree,&mensagem);
    if(resposta == NULL || resposta->opcode == MESSAGE_T__OPCODE__OP_ERROR){
        message_t__free_unpacked(resposta,NULL);
        return -1;
    }
    if(resposta->opcode == (MESSAGE_T__OPCODE__OP_VERIFY + 1)){
        int r = resposta->value;
        message_t__free_unpacked(resposta,NULL);
        return r;
    }

}
