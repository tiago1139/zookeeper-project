#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "network_server.h"
#include "tree_skel.h"

/*
    GRUPO 33
    - Ricardo Soares fc54446
    - Miguel Reis fc54409
    - Tiago Pinto fc55172
*/

char *ip_port;

int main(int argc, char* argv[]){
    // Verifica se foi passado algum argumento
    if (argc != 3){
        printf("Uso: ./tree-server <porto_servidor> <IP:porta zookeeper>\n"); // ja nao precisamos do nr de threads
        printf("Exemplo de uso: ./tree-server 4040 localhost:2181\n");
        return -1;
    }
    char tmp[30] = "127.0.0.1:";
    strcat(tmp, argv[1]);
    ip_port = tmp;
    int listening_socket = network_server_init(atoi(argv[1]));
    printf("Ola Server\n");
    zookeeper_serv_connect(argv[2]);
    tree_skel_init(1); // apenas uma thread secundaria
    int result = network_main_loop(listening_socket);
    tree_skel_destroy();
    printf("skel_destroyed \n");
    network_server_close();
}
