#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include "client_stub.h"

/*
    GRUPO 33
    - Ricardo Soares fc54446
    - Miguel Reis fc54409
    - Tiago Pinto fc55172
*/

extern struct rtree_t *head;
extern struct rtree_t *tail;
extern int fechar;

void pedidos(){
    printf("#### Pedidos permitidos ####\n");
    printf("size , height , get <key> , getkeys , put <key> <data> , getvalues , del <key> , verify <op_n>, quit\n");
}
    
int main(int argc, char* argv[]){
    if (argc != 2){
        printf("Uso: ./tree-client <hostname>:<porto_servidor>\n");
        printf("Exemplo de uso: ./tree-client 192.0.0.0:12345\n");
        return -1;
    }

	
    char *r = strstr(argv[1], ":");
    if(!r) {
        printf("Uso: ./tree-client <hostname>:<porto_servidor>\n");
        printf("Exemplo de uso: ./tree-client 192.0.0.0:12345\n");
        return -1;
    }

    connectZookeeper(argv[1]);

    printf("Ola Cliente\n");
    pedidos();
    while(fechar){
        char buf[60];
        printf("Pedido: ");
        fgets(buf, 60, stdin);
        
        char *command;

        command =  strtok(buf," \n");
        if(strcmp(command,"size")==0){
            printf("comando : %s \n\n", command);
            printf("###################################\n");
            int size = rtree_size(tail);
            if(size < 0 ) {
                printf("Erro ao efetuar operacao de size.\n\n");
            } else {
                printf("size da tree : %d\n\n", size);
            }
            printf("###################################\n");
        }
        else if(strcmp(command,"height")==0){
            printf("comando : %s \n\n", command);
            printf("###################################\n");
            int height = rtree_height(tail);
            printf("height da tree : %d\n\n", height);
            printf("###################################\n");
        }
        else if(strcmp(command,"getkeys")==0){
            char **keys;
            printf("comando : %s \n\n", command);
            printf("###################################\n");
            keys = rtree_get_keys(tail);
            printf("------ All Keys ------\n");
            for (int i = 0; keys[i] != NULL; i++)
            {
                printf("Key -> %s\n", (char*)keys[i]);
            }
            printf("\n###################################\n");
            
	    
            for(int i = 0;keys[i] != NULL; i++){
                free(keys[i]);
            }
            free(keys);
        }
        else if(strcmp(command,"getvalues")==0){
            void**values;
            printf("comando : %s \n\n", command);
            printf("###################################\n");
            values = rtree_get_values(tail);
            printf("------ All Values ------\n");
            for (size_t i = 0; values[i] != NULL; i++)
            {
                printf("Value -> %s\n", (char*)values[i]);
            }
            printf("\n###################################\n");

            
            for (int i = 0; values[i] != NULL;i++){
                free(values[i]);
            }
            free(values);
        }
        else if(strcmp(command,"quit")==0){
            printf("comando : %s \n\n", command);
            rtree_disconnect(tail);
            rtree_disconnect(head);
            zookeeper_client_close();
            exit(0);
            
        }
        else if(strcmp(command,"put")==0){
            char* k = strtok(NULL, " ");
            void* data = strtok(NULL," \n");
            char * t; // first copy the pointer to not change the original
            int size = 0;

            for (t = data; *t != '\0'; t++) {
                size++;
            }
            printf("size : %d\n", size);


            char *key = malloc(strlen(k)+1);
            strcpy(key,k);
            char *data_s = malloc(strlen(data)+1);
            strcpy(data_s,data);
            struct data_t *data_e = data_create2(strlen(data_s)+1,data_s);
            struct entry_t *entry = entry_create(key,data_e);
            printf("comando : %s     key: %s    data: %s \n\n", command, k , (char *)data);
            printf("###################################\n");
            int res = rtree_put(head,entry);
            if(res == -1) {
                printf("Erro ao fazer o pedido! \n\n");
                
            } else {
                printf("Pedido de put do nó com key:%s realizado com o identificador de operação:%d\n\n",key, res);
            }
            printf("###################################\n");

            entry_destroy(entry);

        }
        else if(strcmp(command,"get")==0){
            char* k = strtok(NULL, " \n");
            char *key = malloc(strlen(k)+1);
            strcpy(key,k);
            printf("comando : %s     key: %s \n\n", command, k);
            printf("###################################\n");
            struct data_t *data = rtree_get(tail,key);
            if(data != NULL){
                printf("Size : %d\n", data->datasize -1);
                printf("Data : %s\n\n", (char*)data->data);
                printf("###################################\n");
                data_destroy(data);
            }
            free(key);
            
        }
        else if(strcmp(command,"del")==0){
            char* k = strtok(NULL, " \n");
            char *key = malloc(strlen(k)+1);
            strcpy(key,k);
            printf("comando : %s     key: %s \n\n", command, k);
            printf("###################################\n");
            int res = rtree_del(head,key);
            if(res == -1) {
                printf("Erro ao eliminar o nó/ nó já eliminado/ nó não existente com key %s\n\n", key);
                
            } else {
                printf("Pedido de del do nó com key:%s realizado com o identificador de operação:%d\n\n",key, res);
            }
            free(key);
            printf("###################################\n");
        }
        else if (strcmp(command,"verify")==0)
        {
           char *op = strtok(NULL, " \n");
           int c = rtree_verify(tail,atoi(op));
           if(c == -1){
            printf("Ocorreu um erro ao fazer a operação verify \n\n");
           }
           else {
            if(c == 0){
                printf("Operação %d ainda não foi executada, verifique mais tarde! \n \n",atoi(op));
            } if(c == 1){
                printf("Operação %d verificada, já foi executada! \n\n", atoi(op));
            }
           }
        }
        else{
            printf("comando não válido \n\n");
            pedidos();
        }
    
    }
    rtree_disconnect(tail);
    rtree_disconnect(head);
    zookeeper_client_close();
    exit(0);
    
}    

