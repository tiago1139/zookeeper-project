#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "tree.h"


struct tree_t {
	struct entry_t *entry;
	struct tree_t *right;
	struct tree_t *left;
};

struct tree_t* minValueNode(struct tree_t* node);

struct tree_t *delete_tree_node(struct tree_t *tree, char *key);

void aux_tree_get_keys(struct tree_t *tree, char **allkeys);

void aux_tree_get_values(struct tree_t *tree,void **allvalues);

void print2DUtil(struct tree_t *root, int space);

void print2D(struct tree_t* root);

#endif
