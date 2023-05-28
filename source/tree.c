#include "data.h"
#include "entry.h"
#include "tree-private.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define COUNT  15

/*
    GRUPO 33
    - Ricardo Soares fc54446
    - Miguel Reis fc54409
    - Tiago Pinto fc55172
*/


struct tree_t; /* A definir pelo grupo em tree-private.h */

/* Função para criar uma nova árvore tree vazia.
 * Em caso de erro retorna NULL.
 */
struct tree_t *tree_create(){
    size_t treeSize = sizeof(struct tree_t);
    struct tree_t *tree = memset(malloc(treeSize),0,treeSize);
    if(tree == NULL){
        free(tree);
        return NULL;
    }
    
    tree->left = NULL;
    tree->right = NULL;
    return tree;
}

/* Função para libertar toda a memória ocupada por uma árvore.
 */
void tree_destroy(struct tree_t *tree) {
    if (tree != NULL){
        entry_destroy(tree->entry);
        tree_destroy(tree->left);
        tree_destroy(tree->right);
    }
    free(tree);
}

/* Função para adicionar um par chave-valor à árvore.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na árvore,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
int tree_put(struct tree_t *tree, char *key, struct data_t *value){
    if(tree == NULL){
        tree = tree_create();
    }
    char *key2 = malloc(strlen(key) +1);
    strcpy(key2,key);
    struct data_t *data2 = data_dup(value);
    struct entry_t *entry_new = entry_create(key2,data2);
    if(tree->entry == NULL){
        tree->entry = entry_dup(entry_new);
        entry_destroy(entry_new);
        return 0;
    } else {
        if(entry_compare(tree->entry,entry_new) == 0){
            struct data_t *data_new = data_dup(value);
            char* key_new = malloc(strlen(key) +1);
            strcpy(key_new,key);
            entry_replace(tree->entry,key_new,data_new);
            entry_destroy(entry_new);
            
            return 0;
        } else if(entry_compare(tree->entry,entry_new) == -1){
            entry_destroy(entry_new);
            if(tree->right == NULL){
                tree->right = tree_create();
            }
            return tree_put(tree->right,key,value);
        } else { 
            entry_destroy(entry_new);
            if(tree->left == NULL){
                tree->left = tree_create();
            }
            return tree_put(tree->left,key,value);
        }
    }
    return -1;
}

/* Função para obter da árvore o valor associado à chave key.
 * A função deve devolver uma cópia dos dados que terão de ser
 * libertados no contexto da função que chamou tree_get, ou seja, a
 * função aloca memória para armazenar uma *CÓPIA* dos dados da árvore,
 * retorna o endereço desta memória com a cópia dos dados, assumindo-se
 * que esta memória será depois libertada pelo programa que chamou
 * a função. Devolve NULL em caso de erro.
 */
struct data_t *tree_get(struct tree_t *tree, char *key){
   if(tree == NULL || tree->entry == NULL || key == NULL){
        return NULL;
    } else {
        char *key2 = malloc(strlen(key) +1);
        strcpy(key2,key);
        struct entry_t *entryC = entry_create(key2,NULL);
        if(entry_compare(tree->entry,entryC) == 0){
            entry_destroy(entryC);
            struct data_t *result = data_dup(tree->entry->value);
            
            return result;
        } else if(entry_compare(tree->entry,entryC) == -1){
            entry_destroy(entryC);
            return tree_get(tree->right,key);
        } else { //if(entry_compare(tree->entry,entryC) == 1)
            entry_destroy(entryC);
            return tree_get(tree->left,key);
        }
    }
    printf("deu null\n");
    return NULL;
}

struct tree_t* minValueNode(struct tree_t* node)
{
    struct tree_t* current = node;
  
    
    while (current && current->left != NULL)
        current = current->left;
  
    return current;
}

/* Função que devolve o número de elementos contidos na árvore.
 */
int tree_size(struct tree_t *tree){
    if(tree == NULL || tree->entry == NULL){
        return 0;
    } else if(tree != NULL && tree->entry!= NULL){
        return 1 + tree_size(tree->left) + tree_size(tree->right);
    }
}

// ESTE HEIGHT ESTA A FUNCIONAR
// Find height of a tree, defined by the root node
int tree_height(struct tree_t *tree) {
    if (tree == NULL) 
        return 0;
    else {
        // Find the height of left, right subtrees
        int left_height = tree_height(tree->left);
        int right_height = tree_height(tree->right);
         
        // Find max(subtree_height) + 1 to get the height of the tree
        if (left_height >= right_height)
            return left_height + 1;
        else
            return right_height + 1;
    }
}

/* Função que devolve a altura da árvore.
 */
/*
int tree_height(struct tree_t *tree){
    if(tree == NULL || tree->entry == NULL){
        return 0;
    } else if (tree != NULL && tree->entry != NULL){
        if(tree->left != NULL){
            return 1 + tree_height(tree->left);
        }else{
            return 1;
        }
        if(tree->right != NULL){
            return 1+ tree_height(tree->right);
        }else {
            return 1;
        }
        int left = tree_height(tree->left);
        int right = tree_height(tree->right);
        if(left > right){
            return left+1;
        } else {
            return right +1;
        }
    }   
}
*/
struct tree_t *delete_tree_node(struct tree_t *tree, char *key) {
    // base case
    if (tree == NULL || tree->entry == NULL){
        return tree;
    }
    char *key2 = malloc(strlen(key) +1);
    strcpy(key2,key);
    struct entry_t *entryC = entry_create(key2,NULL);
    if (entry_compare(tree->entry,entryC) == 1) {
        entry_destroy(entryC);
        tree->left = delete_tree_node(tree->left, key);
        
    }
  

    else if (entry_compare(tree->entry,entryC) == -1) {
        entry_destroy(entryC);
        tree->right = delete_tree_node(tree->right, key);
        
    }
    else {
        entry_destroy(entryC);  
        if (tree->left == NULL) {
            printf("a eliminar: (%s, %s)\n", tree->entry->key, (char *)tree->entry->value->data);
            struct tree_t* temp = tree->right;
            entry_destroy(tree->entry);
            free(tree);
            return temp;
        }
        else if (tree->right == NULL) {
            printf("a eliminar: (%s, %s)\n", tree->entry->key, (char *)tree->entry->value->data);
            struct tree_t* temp = tree->left;
            entry_destroy(tree->entry);
            free(tree); 
            return temp;
        }
  
       
        struct tree_t* temp = minValueNode(tree->right);
  
        struct entry_t *entry_temp = entry_dup(temp->entry);
        
  
        tree->right = delete_tree_node(tree->right, temp->entry->key);
        entry_replace(tree->entry, entry_temp->key, entry_temp->value);
        free(entry_temp);
        
    }
    
    return tree;
}


/* Função para remover um elemento da árvore, indicado pela chave key,
 * libertando toda a memória alocada na respetiva operação tree_put.
 * Retorna 0 (ok) ou -1 (key not found).
 */
int tree_del(struct tree_t *tree, char *key){
    struct data_t *data = tree_get(tree,key);
    if(data == NULL){
        return -1;
    }
    data_destroy(data);
    tree = delete_tree_node(tree, key);
    return 0;

}

/* Função que devolve um array de char* com a cópia de todas as keys da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária. As keys devem vir ordenadas segundo a ordenação lexicográfica das mesmas.
 */
char **tree_get_keys(struct tree_t *tree){
    int size = tree_size(tree);
    char **allkeys = calloc(size+1,sizeof(char*));
    if(allkeys == NULL){
        return NULL;
    }
    aux_tree_get_keys(tree,allkeys);
    allkeys[size] = NULL;
    return allkeys;
}

void aux_tree_get_keys(struct tree_t *tree, char **allkeys){
    if(tree != NULL){
        aux_tree_get_keys(tree->left,allkeys);
        char *key = malloc(strlen(tree->entry->key) +1);
        if(key == NULL){
            return;
        }

        strcpy(key,tree->entry->key);
        int i = 0;
        while(allkeys[i]!=NULL){
            i++;
        }
        allkeys[i] = key;
        aux_tree_get_keys(tree->right,allkeys);
    }
}


/* Função que devolve um array de void* com a cópia de todas os values da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 */
void **tree_get_values(struct tree_t *tree){
    int size = tree_size(tree);
    void **allvalues = calloc(size+1,sizeof(struct data_t*));
    if(allvalues == NULL){
        return NULL;
    }
    aux_tree_get_values(tree,allvalues);
    allvalues[size] = NULL;
    return allvalues;
}

void aux_tree_get_values(struct tree_t *tree,void **allvalues){
    if(tree != NULL){
        aux_tree_get_values(tree->left,allvalues);
        void *value = data_dup(tree->entry->value);
        if(value == NULL){
            return;
        }
        
        int i = 0;
        while(allvalues[i]!=NULL){
            i++;
        }
        allvalues[i] = value;
        
        aux_tree_get_values(tree->right,allvalues);
    }
}


/* Função que liberta toda a memória alocada por tree_get_keys().
 */
void tree_free_keys(char **keys){
    int i = 0;
    while(keys[i] != NULL){
        free(keys[i]);
        i++;
    }
    free(keys);
}

/* Função que liberta toda a memória alocada por tree_get_values().
 */
void tree_free_values(void **values){
    int i = 0;
    while(values[i] != NULL){
        data_destroy(values[i]);
        i++;
    }
    free(values);
}

 

void print2DUtil(struct tree_t *root, int space)
{
    // Base case
    if (root == NULL)
        return;
 
    // Increase distance between levels
    space += COUNT;
 
    // Process right child first
    print2DUtil(root->right, space);
 
    // Print current node after space
    // count
    printf("\n");
    for(int i = COUNT; i < space;i++){
        printf(" ");
    }
    printf("(%s, %s)\n", root->entry->key, (char*)root->entry->value->data);
 
    // Process left child
    print2DUtil(root->left, space);
}
 
// Wrapper over print2DUtil()
void print2D(struct tree_t* root)
{
   // Pass initial space count as 0
   print2DUtil(root, 0);
}
