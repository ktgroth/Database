
#ifndef GDB_BPTREE_
#define GDB_BPTREE_

#include "types.h"
#include "column.h"
#include "datablock.h"
#include "dataframe.h"


typedef struct bptree_node
{
    size_t                u;
    int                   is_leaf;
    size_t                nkeys;
    void                **keys;
    datablock_t         **dptr;
    struct bptree_node  **children;
    struct bptree_node   *next;
} bptree_node_t;

typedef struct 
{
    size_t                u;
    size_t                ncols;
    char                **colnames;
    type_e               *coltypes;

    char                 *pkname;
    type_e                pktype;
    bptree_node_t        *root;
    bptree_node_t        *first_leaf;
} bptree_t;


bptree_node_t *init_bptree_node(int is_leaf, size_t u);
void free_bptree_node(bptree_node_t *node);
void print_bptree_node(const bptree_node_t *node, type_e type);


bptree_t *init_bptree(size_t u, size_t ncols, char **colnames, type_e *coltypes,
        char *pkname, type_e pktype);
void free_bptree(bptree_t *tree);
void print_bptree(const bptree_t *tree);


int bptree_insert(bptree_t *tree, void *key, void **values);
int bptree_add(bptree_t *tree, void *key, datablock_t *block);
int bptree_remove_key(bptree_t *tree, void *key);
int bptree_remove(bptree_t *tree, const char *colname, void *value);
const dataframe_t *bptree_lookup_key(const bptree_t *tree, void *key);
const dataframe_t *bptree_lookup(const bptree_t *tree, const char *colname, void *value);
int bptree_update_key(bptree_t *tree, const void *key, size_t colidx, void *value);
int bptree_update(bptree_t *tree, const char *keycol, const void *keyval, const char *colname, void *value);

#endif

