
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/datablock.h"


datablock_t *init_block(size_t ncols, const char **colnames, const type_e *coltypes, void **values)
{
    datablock_t *block = (datablock_t *)calloc(1, sizeof(datablock_t));
    if (!block)
    {
        perror("block = calloc(1, sizeof(datablock_t))");
        return NULL;
    }

    block->ncols = ncols;
    block->cols = (column_t **)calloc(ncols, sizeof(column_t *));
    for (size_t i = 0; i < ncols; ++i)
        block->cols[i] = init_column(colnames[i], init_field(coltypes[i], values[i]));

    return block;
}

void free_block(datablock_t *block)
{
    if (!block)
        return;

    for (size_t i = 0; i < block->ncols; ++i)
        free_column(block->cols[i]);
    free(block);
}

int block_update(datablock_t *block, const char *colname, void *value)
{
    if (!block || !colname || !value)
        return 0;

    size_t idx = -1;
    for (size_t i = 0; i < block->ncols; ++i)
    {
        if (!strcmp(block->cols[i]->name, colname))
        {
            idx = i;
            break;
        }
    }

    if (idx == -1)
        return 0;

    return column_update(block->cols[idx], value);
}

