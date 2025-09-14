
#ifndef GDB_STORAGE_
#define GDB_STORAGE_

#include "types.h"
#include "column.h"
#include "datablock.h"
#include "dataframe.h"
#include "btree.h"


typedef enum
{
    STORAGE_FRAME,
    STORAGE_BTREE,
} storage_type_e;

typedef struct
{
    storage_type_e type;
    union
    {
        dataframe_t *frame;
        btree_t     *btree;
    };
} storage_t;


storage_t           *init_storage(storage_type_e type, size_t ncols, const char **colnames, const type_e *coltypes,
        const char *pkname, const type_e pktype);
void                 free_storage(storage_t *store);


int                  storage_insert(storage_t *store, const void *key, void **value);
int                  storage_add(storage_t *store, const void *key, datablock_t *block);
int                  storage_remove(storage_t *store, const char *colname, void *value);
const dataframe_t   *storage_lookup(const storage_t *store, const char *colname, void *value);
int                  storage_update(storage_t *store, const char *keycol, const void *keyval, const char *colname, void *value);

#endif

