
#ifndef GDB_DATABLOCK_
#define GDB_DATABLOCK_

#include "column.h"


typedef struct
{
    size_t        ncols;
    column_t    **cols;
} datablock_t;


datablock_t *init_block(size_t ncols, const char **colnames, const type_e *coltypes, void **values);
void         free_block(datablock_t *block);


int          block_update(datablock_t *block, const char *colname, void *value);

#endif

