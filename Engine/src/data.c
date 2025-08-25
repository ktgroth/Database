
#include <stdio.h>
#include <stdlib.h>

#include "include/data.h"

datablock_t *init_block(size_t ncols, char **cols, char **colt, void *data)
{
    datablock_t *block = (datablock_t *)calloc(1, sizeof(datablock_t));
    if (!block)
    {
        perror("block = calloc(1, sizeof(datablock_t))");
        return NULL;
    }

    block->ncols = ncols;
    block->cols = (char **)calloc(ncols, sizeof(char **));
    block->colt = (char **)calloc(ncols, sizeof(char **));
    block->data = (void *)calloc(ncols, sizeof(void *));
    memcpy(block->data, data, ncols*sizeof(void *));

    for (size_t i = 0; i < ncols; ++i)
    {
        block->cols[i] = (char *)calloc(strlen(cols[i]), sizeof(char));
        strcpy(block->cols[i], cols[i]);

        block->colt[i] = (char *)calloc(strlen(colt[i]), sizeof(char));
        strcpy(block->colt[i], colt[i]);
    }

    return block;
}

dataframe_t *init_frame(size_t ncols, size_t nentries, char **cols, char **colt, void **data)
{
    dataframe_t *frame = (dataframe_t *)calloc(1, sizeof(dataframe_t));
    if (!frame)
    {
        perror("frame = calloc(1, sizeof(dataframe_t))");
        return NULL;
    }

    frame->ncols = ncols;
    frame->cols = (char **)calloc(ncols, sizeof(char **));
    frame->colt = (char **)calloc(ncols, sizeof(char **));
    frame->data = (void **)calloc(nentries, sizeof(void *));

    for (size_t i = 0; i < ncols; ++i)
    {
        frame->cols[i] = (char *)calloc(strlen(cols[i]), sizeof(char));
        strcpy(frame->cols[i], cols[i]);

        frame->colt[i] = (char *)calloc(strlen(colt[i]), sizeof(char));
        strcpy(frame->colt[i], colt[i]);
    }

    for (size_t i = 0; i < nentries; ++i)
        memcpy(frame->data[i], data[i], ncols*sizeof(void *));

    return frame;
}

void block_change(datablock_t *block, db_id_t, datablock_t *data)
{

}

