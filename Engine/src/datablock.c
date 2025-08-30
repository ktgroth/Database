
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "include/datablock.h"


typedef struct dataframe
{
    size_t          ncols;
    size_t          nrows;
    const char      **colnames;
    const type_e    *coltypes;
    datablock_t     **rows;
} dataframe_t;

datablock_t *init_block(const dataframe_t *frame, void **values)
{
    if (!frame || !values)
        return NULL;

    datablock_t *block = (datablock_t *)calloc(1, sizeof(datablock_t));
    if (!block)
    {
        perror("block = calloc(1, sizeof(datablock_t))");
        return NULL;
    }

    block->ncols = frame->ncols;
    block->cols = (column_t *)calloc(frame->ncols, sizeof(column_t));
    if (!block->cols)
    {
        free(block);
        return NULL;
    }

    for (size_t i = 0; i < frame->ncols; ++i)
    {
        block->cols[i].name = frame->colnames[i];
        block->cols[i].type = frame->coltypes[i];
        block->cols[i].value = values[i];
    }

    return block;
}

void free_block(datablock_t *block)
{
    if (!block)
        return;

    free(block->cols);
    free(block);
}

void print_block(const datablock_t *block)
{
    if (!block)
        return;

    for (size_t i = 0; i < block->ncols; ++i)
    {
        printf("%-15s\t", block->cols[i].name);
        print_value(block->cols[i].type, block->cols[i].value);
    }
}

