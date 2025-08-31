
#ifndef GDB_DATABLOCK_
#define GDB_DATABLOCK_

#include <stddef.h>

#include "column.h"


typedef struct dataframe dataframe_t;

typedef struct
{
    size_t      ncols;
    column_t    *cols;
} datablock_t;


datablock_t *init_block(const dataframe_t *frame, void **values);
void free_block(datablock_t *block);
void print_block(const datablock_t *block);


int block_update(datablock_t *block, const char *col, const void *new_value);

#endif

