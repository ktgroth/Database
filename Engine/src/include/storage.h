
#ifndef GDB_STORAGE_
#define GDB_STORAGE_

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
    } storage;
} storage_t;


storage_t           *init_storage(storage_type_e type);
void                 free_storage(storage_t *store);


int                  storage_add(storage_t *store, );
int                  storage_remove(storage_t *store, );
const dataframe_t   *storage_lookup(const storage_t *store, );
int                  storage_update(storage_t *store, );

#endif

