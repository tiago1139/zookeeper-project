#include "network_client.h"
#include "client_stub-private.h"
#include "message-private.h"

#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "inet.h"

/*
    GRUPO 33
    - Ricardo Soares fc54446
    - Miguel Reis fc54409
    - Tiago Pinto fc55172
*/

/** * * * * * * * * * * * * * * *
*                               *
*       FUNCOES / METODOS       *
*                               *
*                               *
* * * * * * * * * * * * * * * * */
int fechar = 1;

void handler_client(){
	printf("Servidor foi fechado, a fechar o cliente \n");
	fechar = 0; // unsure
}



/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) a base da
 *   informação guardada na estrutura rtree;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtree;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtree_t *rtree) {
	
    /* Verificar parâmetro da função e alocação de memória */
	if (rtree == NULL || rtree->address_port == NULL)
		return -1;

	char *address = strdup(rtree->address_port);
	char *token = strtok(address, ":");
	
	char *port = strtok(NULL, ":");


    int sockfd;

	// Cria socket TCP
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Erro ao criar socket");
		free(address);
		return -1;
	}

    struct sockaddr_in server;
	// Preenche estrutura server para estabelecer conexão
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(port));
	if (inet_pton(AF_INET, address, &server.sin_addr) < 1)
	{
		printf("Erro ao converter IP\n");
		free(address);
		close(sockfd);
		return -1;
	}
	// Estabelece conexão com o servidor definido em server
	if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Erro ao conectar-se ao servidor");
		free(address);
		close(sockfd);
		return -1;
	}

	signal(SIGPIPE,handler_client);
    rtree->socket_desc = sockfd;
	free(address);
	return 0;
}

/* Esta função deve:
 * - Obter o descritor da ligação (socket) da estrutura rtree_t;
 * - Serializar a mensagem contida em msg;
 * - Enviar a mensagem serializada para o servidor;
 * - Esperar a resposta do servidor;
 * - De-serializar a mensagem de resposta;
 * - Retornar a mensagem de-serializada ou NULL em caso de erro.
 */
MessageT *network_send_receive(struct rtree_t * rtree, MessageT *msg) {
	
	if(rtree == NULL){
		perror("rTree esta a NULL");
		return NULL;
	}

	if(msg == NULL){
		perror("msg esta a NULL");
		return NULL;
	}
	
	int sockfd = rtree->socket_desc;

	int buffer_size, serialized_size, result;

	buffer_size = message_t__get_packed_size(msg);

	if (buffer_size < 0) {
		printf("buffer size negativo\n");
		close(sockfd);
		return NULL;
	}

	serialized_size = htonl(buffer_size);
	if ((result = write_all(sockfd, (char *)&serialized_size, 4)) != 4)
	{
		perror("Erro ao enviar dados ao servidor");
		network_close(rtree);
		return NULL;
	}

	char* serialized_msg = malloc(buffer_size);
	if(serialized_msg == NULL){
		perror("Erro ao alocar buffer de serializacao");
		return NULL;
	}
	message_t__pack(msg,serialized_msg);

	write_all(sockfd, serialized_msg, buffer_size);
	printf("A espera do Servidor ...\n");
	if ((result = read_all(sockfd, (char *)&serialized_size, 4)) == 0) {
		perror("O servidor desligou-se");
		network_close(rtree);
		free(serialized_msg);
		return NULL;
	}
	if (result != 4) {
		perror("Erro ao receber dados do servidor");
		network_close(rtree);
		free(serialized_msg);
		return NULL;
	}

	buffer_size = ntohl(serialized_size);
	MessageT* resp_deserialized;
	
	char *resp_serialized = malloc(buffer_size );

	read_all(sockfd, resp_serialized, buffer_size);

	resp_deserialized = message_t__unpack(NULL,buffer_size,resp_serialized);

	free(resp_serialized);
	free(serialized_msg);

	return resp_deserialized;

}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtree_t * rtree){
	return close(rtree->socket_desc); // 0 in success, -1 in error
}
