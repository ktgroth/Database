
#include <stdio.h>
#include <stdlib.h>

#include "include/data.h"

datablock_t *init_block(size_t ncols, char **cols, char **colt, void **data)
{
    datablock_t *block = (datablock_t *)calloc(1, sizeof(datablock_t));
    if (!block)
    {
        perror("block = calloc(1, sizeof(datablock_t))");
        return NULL;
    }

    block->ncols = ncols;
    block->cols = (char **)calloc(ncols, sizeof(char *));
    block->colt = (char **)calloc(ncols, sizeof(char *));
    block->data = (void **)calloc(ncols, sizeof(void *));

    for (size_t i = 0; i < ncols; ++i)
    {
        block->cols[i] = strdup(cols[i]);
        block->colt[i] = strdup(colt[i]);
        block->data[i] = data[i];
    }

    return block;
}

dataframe_t *init_frame(size_t ncols, char **cols, char **colt)
{
    dataframe_t *frame = (dataframe_t *)calloc(1, sizeof(dataframe_t));
    if (!frame)
    {
        perror("frame = calloc(1, sizeof(dataframe_t))");
        return NULL;
    }

    frame->ncols = ncols;
    frame->nentries = 0;
    frame->cols = (char **)calloc(ncols, sizeof(char *));
    frame->colt = (char **)calloc(ncols, sizeof(char *));
    frame->data = NULL;

    for (size_t i = 0; i < ncols; ++i)
    {
        frame->cols[i] = strdup(cols[i]);
        frame->colt[i] = strdup(colt[i]);
    }

    return frame;
}

void free_block(datablock_t *block)
{
    if (!block)
        return;

    for (size_t i = 0; i < block->ncols; ++i)
    {
        free(block->cols[i]);
        free(block->colt[i]);
    }

    free(block->cols);
    free(block->colt);
    free(block->data);
    free(block);
}

void free_frame(dataframe_t *frame)
{
    if (!frame)
        return;

    for (size_t i = 0; i < frame->ncols; ++i)
    {
        free(frame->cols[i]);
        free(frame->colt[i]);
    }

    for (size_t i = 0; i < frame->nentries; ++i)
        free(frame->data[i]);

    free(frame->cols);
    free(frame->colt);
    free(frame->data);
    free(frame);
}

void print_block(datablock_t *block)
{
    
}

void print_frame(dataframe_t *frame)
{
    for (size_t i = 0; i < frame->ncols; ++i)
        printf(" %-16s ", frame->cols[i]);
    printf("\n");

    for (size_t i = 0; i < frame->nentries; ++i)
    {
        for (size_t j = 0; j < frame->ncols; ++j)
        {
            if (!strcmp(frame->colt[j], "STRING"))
                printf(" %-16s ", (char *)frame->data[i][j]);
            if (!strcmp(frame->colt[j], "INT"))
                printf(" %-16ld ", (long)frame->data[i][j]);
        }

        printf("\n");
    }
}

void block_change(datablock_t *block, datablock_t *data)
{
    free_block(block);
    block = init_block(data->ncols, data->cols, data->colt, data->data);
}

void frame_add(dataframe_t *frame, datablock_t *data)
{
    frame->data = realloc(frame->data, (frame->nentries + 1) * sizeof(void **));
    frame->data[frame->nentries] = (void **)calloc(frame->ncols, sizeof(void *));
    for (size_t i = 0; i < frame->ncols; ++i)
        frame->data[frame->nentries][i] = data->data[i];

    ++frame->nentries;
}

void frame_remove(dataframe_t *frame, size_t row)
{
    if (row >= frame->nentries)
        return;

    for (size_t i = row; i < frame->nentries - 1; ++i)
        frame->data[i] = frame->data[i + 1];

    --frame->nentries;
    frame->data = realloc(frame->data, frame->nentries * sizeof(void **));
}

void frame_change(dataframe_t *frame, size_t row, datablock_t *data)
{
    if (row >= frame->nentries)
        return;

    for (size_t i = 0; i < frame->ncols; ++i)
        frame->data[row][i] = data->data[i];
}

datablock_t *frame_search(dataframe_t *frame, size_t row)
{
    if (row >= frame->nentries)
        return NULL;

    return init_block(frame->ncols, frame->cols, frame->colt, frame->data[row]);
}

