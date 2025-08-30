
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "include/dataframe.h"


dataframe_t *init_frame(size_t ncols, const char **colnames, const type_e *coltypes)
{
    if (!colnames || !coltypes)
        return NULL;

    dataframe_t *frame = (dataframe_t *)calloc(1, sizeof(dataframe_t));
    if (!frame)
    {
        perror("frame = calloc(1, sizeof(dataframe_t))");
        return NULL;
    }

    frame->ncols = ncols;
    frame->nrows = 0;
    frame->colnames = colnames;
    frame->coltypes = coltypes;
    frame->rows = NULL;

    return frame;
}

void free_frame(dataframe_t *frame)
{
    if (!frame)
        return;

    if (frame->rows)
    {
        for (size_t i = 0; i < frame->nrows; ++i)
            free_block(frame->rows[i]);

        free(frame->rows);
    }

    free(frame);
}

void print_frame(const dataframe_t *frame)
{
    if (!frame)
        return;

    for (size_t i = 0; i < frame->ncols; ++i)
        printf("  %-15s  ", frame->colnames[i]);
    printf("\n");

    for (size_t i = 0; i < frame->nrows; ++i)
    {
        datablock_t *row = frame->rows[i];
        for (size_t j = 0; j < frame->ncols; ++j)
            print_value(row->cols[j].type, row->cols[j].value);

        printf("\n");
    }
}


int frame_add(dataframe_t *frame, const datablock_t *values)
{
    if (!frame || !values)
        return 0;

    datablock_t **new_rows = realloc(frame->rows, (frame->nrows + 1) * sizeof(datablock_t *));
    if (!new_rows)
    {
        perror("new_rows = realloc(frame->rows, (frame->nrows + 1) * sizeof(datablock_t *))");
        return 0;
    }

    frame->rows[frame->nrows] = values;
    ++frame->nrows;
    return 1;
}

const dataframe_t *frame_find(const dataframe_t *frame, const char *keycol, const void *keyval)
{
    if (!frame || !keycol || !keyval)
        return NULL;

    size_t key = SIZE_MAX;
    for (size_t i = 0; i < frame->ncols; ++i)
        if (!strcmp(keycol, frame->colnames[i]))
        {
            key = i;
            break;
        }

    if (key == SIZE_MAX)
        return NULL;

    dataframe_t *found = init_frame(frame->ncols, frame->colnames, frame->coltypes);
    for (size_t i = 0; i < frame->nrows; ++i)
    {
        datablock_t *row = frame->rows[i];
        if (!cmp_value(frame->coltypes[key], row->cols[key].value, keyval))
            frame_add(found, row);
    }

    return found;
}

int frame_update(dataframe_t *frame, const char *keycol, const void *keyval, const char *col, void *new_value)
{
    if (!frame || !keycol || !keyval || !col || !new_value)
        return 0;

    size_t key = SIZE_MAX;
    for (size_t i = 0; i < frame->ncols; ++i)
        if (!strcmp(keycol, frame->colnames[i]))
        {
            key = i;
            break;
        }

    if (key == SIZE_MAX)
        return 0;

    size_t idx = SIZE_MAX;
    for (size_t i = 0; i < frame->ncols; ++i)
        if (!strcmp(col, frame->colnames[i]))
        {
            idx = i;
            break;
        }

    if (idx == SIZE_MAX)
        return 0;

    for (size_t i = 0; i < frame->nrows; ++i)
    {
        datablock_t *row = frame->rows[i];
        if (!cmp_value(frame->coltypes[key], row->cols[key].value, keyval))
            row->cols[idx].value = new_value;
    }

    return 1;
}

int frame_remove(dataframe_t *frame, const char *keycol, const void *keyval)
{
    if (!frame || !keycol || !keyval)
        return 0;

    size_t key = SIZE_MAX;
    for (size_t i = 0; i < frame->ncols; ++i)
        if (!strcmp(keycol, frame->colnames[i]))
        {
            key = i;
            break;
        }

    if (key == SIZE_MAX)
        return 0;

    size_t deleted = 0;
    for (size_t i = 0; i < frame->nrows; ++i)
    {
        datablock_t *row = frame->rows[i];
        if (!cmp_value(frame->coltypes[key], row->cols[key].value, keyval))
        {
            for (size_t f = i; f < frame->nrows - 1; ++f)
                frame->rows[f] = frame->rows[f + 1];
            ++deleted;

            free_block(row);
        }

    }

    datablock_t **new_rows = realloc(frame->rows, (frame->nrows - deleted) * sizeof(datablock_t *));
    if (!new_rows)
    {
        perror("new_rows = realloc(frame->rows, (frame->nrows - deleted) * sizeof(datablock_t *))");
        return 0;
    }

    frame->rows = new_rows;
    frame->nrows -= deleted;

    return 1;
}

