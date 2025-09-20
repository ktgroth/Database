
#ifndef GDB_BTREE_
#define GDB_BTREE_

#include "types.h"
#include "datablock.h"
#include "dataframe.h"

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
    size_t            u;
    size_t            ncols;
    char            **colnames;
    type_e           *coltypes;

    char             *pkname;
    type_e            pktype;
    btree_node_t     *root;
} btree_t;


btree_node_t *init_btree_node(int is_leaf, size_t u);
void free_btree_node(btree_node_t *node);
void print_btree_node(const btree_node_t *node, type_e type);


btree_t *init_btree(size_t u, size_t ncols, char **colnames, type_e *coltypes, char *pkname, type_e pktype);
void free_btree(btree_t *tree);
void print_btree(const btree_t *tree);


int btree_insert(btree_t *tree, void *key, void **values);
int btree_add(btree_t *tree, void *key, datablock_t *block);
int btree_remove_key(btree_t *tree, void *key);
int btree_remove(btree_t *tree, const char *colname, void *value);
const dataframe_t *btree_lookup_key(const btree_t *tree, void *key);
const dataframe_t *btree_lookup(const btree_t *tree, const char *colname, void *value);
int btree_update_key(btree_t *tree, const void *key, size_t colidx, void *value);
int btree_update(btree_t *tree, const char *keycol, const void *keyval, const char *colname, void *value);

#endif

