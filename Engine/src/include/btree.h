
#ifndef GDB_BTREE_
#define GDB_BTREE_

#include "id.h"
#include "data.h"


typedef struct btree
{
    db_id_t *keys;
    datablock_t **dptr;
    size_t nkeys;
    struct btree **children;
    int is_leaf;
    size_t u;
} btree_t;

btree_t *init_btree(int is_leaf, size_t u);
void print_btree(btree_t *root, size_t level);

void btree_add(btree_t **root, db_id_t key, datablock_t *block);
void btree_remove(btree_t **root, db_id_t key);
void btree_change(btree_t **root, db_id_t key, datablock_t *block);
datablock_t *btree_search(btree_t *root, db_id_t key);

#endif

