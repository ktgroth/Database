
#ifndef GDB_TYPES_
#define GDB_TYPES_

#include "dataframe.h"
#include "btree.h"


typedef struct
{
    const char       *name;

    size_t            nidxs;
    btree_t         **idxs;

    size_t            ncols;
    const char      **colnames;
    const type_e     *coltypes;

    size_t            nrows;
    int               indexed;
    const char       *pkname;
    const type_e      pktype;
    storage_t        *rows;
};


table_t             *init_table(size_t ncols, const char **colnames, const type_e *coltypes, storage_type_e stype,
                                int indexed, const char *pkname, const type_e pktype);
void                 free_table(table_t *tbl);


int                  table_insert(table_t *tbl, datablock_t *row);
int                  table_remove(table_t *tbl, const char *colname, const void *coltype);
const dataframe_t   *table_lookup(const table_t *tbl, const char *colname, const void *value);
int                  table_update(table_t *tbl, const char *keyname, const void *keyval, const char *colname, const void *value);

#endif

