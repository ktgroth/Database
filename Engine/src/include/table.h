
#ifndef GDB_TABLE_
#define GDB_TABLE_

#include "types.h"
#include "datablock.h"
#include "dataframe.h"
#include "btree.h"
#include "storage.h"


typedef struct
{
    char             *name;
    size_t            indexed;

    size_t            nidxs;
    btree_t         **idxs;

    size_t            ncols;
    char            **colnames;
    type_e           *coltypes;

    size_t            nrows;
    char             *pkname;
    type_e            pktype;
    storage_t        *rows;
} table_t;


table_t             *init_table(char *name, size_t ncols, char **colnames, type_e *coltypes,
                                int indexed, char *pkname, type_e pktype);
void                 free_table(table_t *tbl);
void                 print_table(const table_t *tbl);
void                 print_table_index(const table_t *tbl, const char *colname);


int                  table_add_index(table_t *tbl, const char *colname);
int                  table_remove_index(table_t *tbl, const char *colname);

int                  table_insert(table_t *tbl, void **values);
int                  table_add(table_t *tbl, datablock_t *row);
int                  table_remove(table_t *tbl, const char *colname, void *value);
const dataframe_t   *table_lookup(const table_t *tbl, const char *colname, void *value);
int                  table_update(table_t *tbl, const char *keyname, const void *keyval, const char *colname, void *value);

#endif

