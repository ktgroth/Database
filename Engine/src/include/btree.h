
#ifndef GDB_BTREE_
#define GDB_BTREE_

#include "types.h"
#include "datablock.h"

typedef size_t db_id_t;

typedef struct btree_node
{
    size_t              u;
    int                 is_leaf;
    size_t              nkeys;
    void                **keys;
    datablock_t         **dptr;
    struct btree_node   **children;
} btree_node_t;

typedef struct
{
    size_t          u;
    const char      *colname;
    type_e          coltype;
    btree_node_t    *root;
} btree_t;


btree_node_t *init_btree_node(int is_leaf, size_t u);
void free_btree_node(btree_node_t *node);
void print_btree_node(const btree_node_t *node, type_e type);


btree_t *init_btree(size_t u, const char *colname, const type_e coltype);
void free_btree(btree_t *tree);
void print_btree(const btree_t *tree);


int btree_add(btree_t *tree, const void *key, datablock_t *block);
int btree_remove(btree_t *tree, const void *key);
int btree_update(btree_t *tree, const void *key, const datablock_t *block);
const datablock_t *btree_search(const btree_t *tree, const void *key);

#endif

