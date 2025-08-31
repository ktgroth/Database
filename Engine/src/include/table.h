
#ifndef GDB_TABLE_
#define GDB_TABLE_

#include "dataframe.h"
#include "btree.h"


typedef struct
{
    size_t          ncols;
    size_t          nindexes;
    char            **colnames;
    type_e          *coltypes;
    dataframe_t     *frame;
    int             indexed;
    btree_t         **indexes;
} table_t;

table_t *init_table(size_t ncols, const char **colnames, const type_e *coltypes, int indexed);
void free_table(table_t *tbl);
void print_table(const table_t *tbl);


int table_add(table_t *tbl, datablock_t *block);
const dataframe_t *table_search(const table_t *tbl, size_t ncols, const char **cols, const void **values);
int table_update(table_t *tbl, const char *keycol, const void *keyval, const char *col, void *value);
int table_delete(table_t *tbl, const char *keycol, const void *keyval);

#endif

