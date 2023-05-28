#include "entry.h"
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


/* Função que cria uma entry, reservando a memória necessária para a
 * estrutura e inicializando os campos key e value, respetivamente, com a
 * string e o bloco de dados passados como parâmetros, sem reservar
 * memória para estes campos.

 */
struct entry_t *entry_create(char *key, struct data_t *data){

    size_t dataEntrySize = sizeof(struct entry_t);
    struct entry_t *entryStruct = memset(malloc(dataEntrySize),0,dataEntrySize);
    if(entryStruct == NULL){
        return NULL;
    }

    entryStruct->key = key;
    entryStruct->value = data;
    return entryStruct;
}

/* Função que elimina uma entry, libertando a memória por ela ocupada
 */
void entry_destroy(struct entry_t *entry){
    if(entry == NULL) {
        return;
    }

    if(entry->key != NULL) {
        free(entry->key);
    }
    data_destroy(entry->value);
    free(entry);
}

/* Função que duplica uma entry, reservando a memória necessária para a
 * nova estrutura.
 */
struct entry_t *entry_dup(struct entry_t *entry){
    struct entry_t *temp = malloc(sizeof(struct entry_t));
    if(temp == NULL) {
        return NULL;
    }
    temp->value = data_create(entry->value->datasize);
    temp->key = strdup(entry->key);
    memcpy(temp->value->data, entry->value->data, entry->value->datasize);
    
    return temp;
}

/* Função que substitui o conteúdo de uma entrada entry_t.
*  Deve assegurar que destroi o conteúdo antigo da mesma.
*/
void entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value){
    if(entry == NULL || new_key == NULL || new_value == NULL || new_value->datasize <= 0 || new_value->data == NULL){
        return;
    }
    free(entry->key);
    data_destroy(entry->value);

    entry->key = new_key;
    entry->value = new_value;
}

/* Função que compara duas entradas e retorna a ordem das mesmas.
*  Ordem das entradas é definida pela ordem das suas chaves.
*  A função devolve 0 se forem iguais, -1 se entry1<entry2, e 1 caso contrário.
*/
int entry_compare(struct entry_t *entry1, struct entry_t *entry2){
    int result = strcmp(entry1->key,entry2->key);
    if(result == 0){
        return 0;
    } else if(result < 0){
        return -1;
    } else {
        return 1;
    }
}

