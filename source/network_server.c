#include "tree_skel.h"
#include "inet.h"
#include "network_server.h"
#include "message-private.h"

#include <errno.h>
#include <poll.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>


/*
    GRUPO 33
    - Ricardo Soares fc54446
    - Miguel Reis fc54409
    - Tiago Pinto fc55172
*/



int sockfd;
int sair = 0;

/** * * * * * * * * * * * * * * *
*                               *
*       FUNCOES / METODOS       *
*                               *
*                               *
* * * * * * * * * * * * * * * * */

void handler(){
	printf("\nServidor foi interrompido por um Ctrl+C \n");
	printf("A fechar servidor \n");
	sair = 1;
}


/* Função para preparar uma socket de receção de pedidos de ligação
 * num determinado porto.
 * Retornar descritor do socket (OK) ou -1 (erro).
 */
int network_server_init(short port) {
	int setport = 1;
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR ,&setport,sizeof(int));
	
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("Erro ao criar socket");
        return -1;
    }

    struct sockaddr_in server;

    // Preenche estrutura server para bind
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    // Faz bind
    if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("Erro ao fazer bind");
        close(sockfd);
        return -1;
    };
	
	signal(SIGINT,handler);
    // Faz listen
    if (listen(sockfd, 0) < 0){
        perror("Erro ao executar listen");
        close(sockfd);
        return -1;
    };
	
	


    return sockfd;
}

/* Esta função deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensagem de-serializada ao skeleton para ser processada;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 */

int network_main_loop(int listening_socket){

	int s = 4;
	struct sockaddr_in client;
    socklen_t size_client;
	struct pollfd connections[s];
	int i,result;
	for (i = 0; i < s; i++)
    	connections[i].fd = -1; 

	connections[0].fd = listening_socket;  // Vamos detetar eventos na welcoming socket
  	connections[0].events = POLLIN;  // Vamos esperar ligações nesta socket	

	int nfds = 1; 
	int kfds;

	while(kfds = poll(connections,nfds,50) >= -1 && !sair){
		if(kfds > 0){
			if ((connections[0].revents & POLLIN) && (nfds < s)){ // Pedido na listening socket ?
				printf("client wants to connect\n");
        		if ((connections[nfds].fd = accept(connections[0].fd, (struct sockaddr *) &client, &size_client)) > 0){ // Ligação feita ?
				printf("client connected \n");
          		connections[nfds].events = POLLIN; // Vamos esperar dados nesta socket
          		nfds++;
				}
			}	for (i = 1; i < nfds; i++){ // todos menos o listening
					if (connections[i].revents & POLLIN) { // Dados para ler ?
					MessageT *client_msg = network_receive(connections[i].fd);
						if(client_msg == NULL) {
							close(connections[i].fd);
							connections[i].fd = -1;
							perror("cliente desconectou-se / erro");
						} else {
							printf("invoking \n");
							result = invoke(client_msg);
							printf("invoked \n");
							if(network_send(connections[i].fd, client_msg)==-1){
								close(connections[i].fd);
								connections[i].fd = -1;
							}
						}
					
					} else if((connections[i].revents & POLLHUP) || (connections[i].revents & POLLERR)){
								close(connections[i].fd);
								connections[i].fd = -1;
							}
			}
		}
		int c = 0;
		for(int i = 1; i < nfds;i++){ // -1 2 3 // nfds = 4 -> nfds = 3; 
			if(connections[i].fd == -1){
				for(int j = i; j < nfds;j++){
					if(connections[j].fd != -1){
						connections[i].fd = connections[j].fd;
						connections[j].fd = -1;
						break;
					}
				}
				
			}else{
				c++;
			}
		}
		nfds = c+1;
	
	}
	printf("a sair.\n");
    return 0;

}



/* Esta função deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura message_t.
 */
MessageT *network_receive(int client_socket) {
    int result, serialized_size, buffer_size;
    if ((result = read_all(client_socket, (char *)&serialized_size, 4)) == 0) {
		perror("O cliente desligou-se");
		return NULL;
	}
	if (result != 4) {
		perror("Erro ao receber dados do cliente");
		return NULL;
	}

	buffer_size = ntohl(serialized_size);
	MessageT* resp_deserialized;
	
	char *resp_serialized = malloc(buffer_size);
	read_all(client_socket, resp_serialized, buffer_size);

	resp_deserialized = message_t__unpack(NULL,buffer_size,resp_serialized);

	free(resp_serialized);

    return resp_deserialized;
}

/* Esta função deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Libertar a memória ocupada por esta mensagem;
 * - Enviar a mensagem serializada, através do client_socket.
 */
int network_send(int client_socket, MessageT *msg) {
    int buffer_size, serialized_size, result;
    buffer_size = message_t__get_packed_size(msg);

    if (buffer_size < 0) {
		close(client_socket);
		return -1;
	}

    serialized_size = htonl(buffer_size);
	if ((result = write_all(client_socket, (char *)&serialized_size, 4)) != 4)
	{
		perror("Erro ao enviar dados ao cliente");
		return -1;
	}

	char* serialized_msg = malloc(buffer_size);
	if(serialized_msg == NULL){
		perror("Erro ao alocar buffer de serializacao");
		return -1;
	}
	message_t__pack(msg,serialized_msg);

	write_all(client_socket, serialized_msg, buffer_size);

    
	message_t__free_unpacked(msg,NULL);
	free(serialized_msg);
    return 0;
}

/* A função network_server_close() liberta os recursos alocados por
 * network_server_init().
 */
int network_server_close() {
	
	close(sockfd);
}
