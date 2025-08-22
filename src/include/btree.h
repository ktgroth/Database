
#ifndef GDB_BTREE_
#define GDB_BTREE_

#include "id.h"

typedef struct btree
{
    int *keys;
    size_t nkeys;
    struct btree **children;
    int is_leaf;
    size_t u;
} btree_t;

btree_t *init_btree(int is_leaf, size_t u);
void print_btree(btree_t *root, size_t level);

void btree_add(btree_t **root, int key);
void btree_remove(btree_t **root, int key);
int btree_search(btree_t *root, int key);

#endif

