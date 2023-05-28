#include "data.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

struct data_t *data_create(int size) {

    if(size <= 0){
        return NULL;
    }

   size_t dataStructSize = sizeof(struct data_t);
   struct data_t *data = memset(malloc(dataStructSize),0,dataStructSize);

    if(data == NULL){
        return NULL;
    }
    

    data->datasize = size;
    data->data = memset(malloc(size),0,size);

    if(data->data == NULL){
        return NULL;
    }

    return data;

}

struct data_t *data_create2(int size, void *data) {

    if(size <= 0 || data == NULL){
        return NULL;
    }

    size_t dataStructSize = sizeof(struct data_t);
    struct data_t *dataStruct = memset(malloc(dataStructSize),0,dataStructSize);

    if(dataStruct == NULL){
        return NULL;
    }

    dataStruct->datasize = size;
    dataStruct->data = data;

    return dataStruct;
}


void data_destroy(struct data_t *data) {
    if(data == NULL) {
        return;
    }
    if(data->data != NULL) {
        free(data->data);
    }
    free(data);
}

struct data_t *data_dup(struct data_t *data) {

    if(data == NULL || (data->datasize <= 0) || (data->data == NULL)) {
        return NULL;
    }
    
    struct data_t *result = data_create(data->datasize);

    memcpy(result->data, data->data, data->datasize);

    return result;
}
/* Função que substitui o conteúdo de um elemento de dados data_t.
*  Deve assegurar que destroi o conteúdo antigo do mesmo.
*/
void data_replace(struct data_t *data, int new_size, void *new_data){
    if(data == NULL || new_size <= 0 || new_data == NULL || data->datasize <= 0){
        return;
    }
    if(data->data != NULL || data != NULL || data->datasize > 0){
        free(data->data);
    }
    data->datasize = new_size;
    data->data = new_data;

}

