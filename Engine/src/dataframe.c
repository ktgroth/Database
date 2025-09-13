
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/dataframe.h"


datablock_t *construct_block(const dataframe_t *frame, const void **values)
{
    return init_block(frame->ncols, frame->colnames, frame->coltypes, values);
}

dataframe_t *init_frame(size_t ncols, const char **colnames, const type_e *coltypes)
{
    dataframe_t *frame = (dataframe_t *)calloc(1, sizeof(dataframe_t));
    if (!frame)
    {
        perror("frame = calloc(1, sizeof(dataframe_t))");
        return NULL;
    }

    frame->ncols = ncols;
    frame->colnames = colnames;
    frame->coltypes = coltypes;
    frame->nrows = 0;
    frame->capacity = 0;
    frame->rows = NULL;

    return frame;
}

void free_frame(dataframe_t *frame)
{
    if (!frame)
        return;

    for (size_t i = 0; i < frame->nrows; ++i)
        free_block(frame->rows[i]);
    free(frame->rows);

    free(frame);
}

void print_frame(const dataframe_t *frame)
{
    if (!frame)
        return;

    for (size_t i = 0; i < frame->ncols; ++i)
        printf(" %-15s ", frame->colnames[i]);
    puts("");

    for (size_t i = 0; i < frame->nrows; ++i)
    {
        datablock_t *row = frame->rows[i];
        for (size_t j = 0; j < frame->ncols; ++j)
            print_value(row->cols[j]->field->type, row->cols[j]->field->value);

        puts("");
    }
}

int frame_insert(dataframe_t *frame, const void **values)
{
    if (!frame || !values)
    {
        perror("!frame || !values");
        return 0;
    }

    datablock_t *block = construct_block(frame, values);
    if (!frame_add(frame, block))
    {
        free_block(block);
        return 0;
    }

    return 1;
}

int frame_add(dataframe_t *frame, const datablock_t *block)
{
    if (!frame || !block)
    {
        perror("!frame || !block");
        return 0;
    }

    if (frame->nrows == frame->capacity)
    {
        datablock_t **new_rows = realloc(frame->rows, ((3 * frame->capacity) + 1) * sizeof(datablock_t *));
        if (!new_rows)
        {
            perror("!new_rows");
            return 0;
        }

        frame->capacity = (3 * frame->capacity) + 1;
        frame->rows = new_rows;
    }

    frame->rows[frame->nrows] = block;
    ++frame->nrows;

    return 1;
}

int frame_remove(dataframe_t *frame, const char *colname, const void *value)
{
    if (!frame || !colname || !value)
    {
        perror("!frame || !colname || !value");
        return 0;
    }

    size_t idx = -1;
    for (size_t i = 0; i < frame->ncols; ++i)
    {
        if (!strcmp(colname, frame->colnames[i]))
        {
            idx = i;
            break;
        }
    }

    if (idx == -1)
        return 0;

    for (size_t i = 0; i < frame->nrows; ++i)
    {
        datablock_t *row = frame->rows[i];
        if (column_cmp(row->cols[idx], value))
        {
            for (size_t j = i; j < frame->nrows - 1; ++j)
                frame->rows[j] = frame->rows[j + 1];
            --i;
        }
    }

    return 1;
}

const dataframe_t *frame_lookup(const dataframe_t *frame, const char *colname, const void *value)
{
    if (!frame || !colname || !value)
    {
        perror("!frame || !colname || !value");
        return NULL;
    }

    size_t idx = -1;
    for (size_t i = 0; i < frame->ncols; ++i)
        if (!strcmp(frame->colnames[i], colname))
        {
            idx = i;
            break;
        }

    if (idx == -1)
        return NULL;

    dataframe_t *lookup = init_frame(frame->ncols, frame->colnames, frame->coltypes);
    for (size_t i = 0; i < frame->nrows; ++i)
    {
        datablock_t *row = frame->rows[i];
        if (!column_cmp_value(row->cols[idx], value))
            frame_add(lookup, row);
    }

    return lookup;
}

int frame_update(dataframe_t *frame, const char *keyname, const void *keyval, const char *colname, const void *value)
{
    if (!frame || !keyname || !keyval || !colname || !value)
        return 0;

    size_t kidx = -1, vidx = -1;
    for (size_t i = 0; i < frame->ncols; ++i)
        if (!strcmp(frame->colnames[i], keyname))
        {
            kidx = i;
            break;
        }

    for (size_t i = 0; i < frame->ncols; ++i)
        if (!strcmp(frame->colnames[i], colname))
        {
            vidx = i;
            break;
        }

    if (kidx == -1 || vidx == -1)
        return 0;

    dataframe_t *lookup = init_frame(frame->ncols, frame->colnames, frame->coltypes);
    for (size_t i = 0; i < frame->nrows; ++i)
    {
        datablock_t *row = frame->rows[i];
        if (!column_cmp_value(row->cols[kidx], keyval))
        {
            column_update(row->cols[vidx], value);
        }
    }


    return 1;
}

