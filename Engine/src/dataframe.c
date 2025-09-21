
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/dataframe.h"


static datablock_t *construct_block(const dataframe_t *frame, void **values)
{
    return init_block(frame->ncols, frame->colnames, frame->coltypes, values);
}

dataframe_t *init_frame(size_t ncols, char **colnames, type_e *coltypes)
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

int frame_insert(dataframe_t *frame, void **values)
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

#define MIN(x, y) ((x) > (y) ? y : x) 

static int check_cols(dataframe_t *frame, datablock_t *block)
{
    for (size_t i = 0; i < MIN(frame->ncols, block->ncols); ++i)
        if (strcmp(frame->colnames[i], block->cols[i]->name))
            return 0;

    if (frame->ncols != block->ncols)
        return 0;

    return 1;
}

static column_t **update_column_metadata(const datablock_t *orig, const dataframe_t *frame)
{
    size_t ncols = frame->ncols;
    column_t **new_cols = calloc(ncols, sizeof(column_t *));
    if (!new_cols)
        return NULL;

    for (size_t i = 0; i < ncols; ++i)
    {
        char *colname = frame->colnames[i];
        column_t *src_col = NULL;
        for (size_t j = 0; j < orig->ncols; ++j)
        {
            if (!strcmp(orig->cols[j]->name, colname))
            {
                src_col = orig->cols[j];
                break;
            }
        }

        datafield_t *new_field = NULL;
        if (src_col)
            new_field = init_field(src_col->field->type, copy_key_value(src_col->field->type, src_col->field->value));
        else
            new_field = init_field(frame->coltypes[i], NULL);

        if (!new_field)
        {
            for (size_t k = 0; k < i; ++k)
                free_column(new_cols[k]);
            free(new_cols);
            return NULL;
        }

        new_cols[i] = init_column(colname, new_field);
        if (!new_cols[i])
        {
            free_field(new_field);
            for (size_t k = 0; k < i; ++k)
                free_column(new_cols[k]);
            free(new_cols);
            return NULL;
        }
    }

    return new_cols;
}

static datablock_t *update_block_structure(const datablock_t *orig, const dataframe_t *frame)
{
    if (!orig || !frame)
        return NULL;

    column_t **new_cols = update_column_metadata(orig, frame);
    if (!new_cols)
        return NULL;

    datablock_t *new_block = (datablock_t *)calloc(1, sizeof(datablock_t));
    if (!new_block)
    {
        perror("new_block = calloc(1, sizeof(datablock_t))");
        for (size_t i = 0; i < frame->ncols; ++i)
            free_column(new_cols[i]);
        free(new_cols);
        return NULL;
    }

    new_block->ncols = frame->ncols;
    new_block->cols = new_cols;

    return new_block;
}

int frame_add(dataframe_t *frame, datablock_t *block)
{
    if (!frame || !block)
    {
        perror("!frame || !block");
        return 0;
    }

    if (!check_cols(frame, block))
    {
        datablock_t *new_block = update_block_structure(block, frame);
        if (!new_block)
            return 0;

        free_block(block);
        block = new_block;
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

int frame_remove(dataframe_t *frame, const char *colname, void *value)
{
    if (!frame || !colname || !value)
    {
        perror("!frame || !colname || !value");
        return 0;
    }

    size_t idx = -1ULL;
    for (size_t i = 0; i < frame->ncols; ++i)
    {
        if (!strcmp(colname, frame->colnames[i]))
        {
            idx = i;
            break;
        }
    }

    if (idx == -1ULL)
        return 0;

    for (size_t i = 0; i < frame->nrows; ++i)
    {
        datablock_t *row = frame->rows[i];
        if (!column_cmp_value(row->cols[idx], value))
        {
            for (size_t j = i; j < frame->nrows - 1; ++j)
                frame->rows[j] = frame->rows[j + 1];
            --i;
            --frame->nrows;
        }
    }

    return 1;
}

const dataframe_t *frame_lookup(const dataframe_t *frame, const char *colname, void *value)
{
    if (!frame || !colname || !value)
    {
        perror("!frame || !colname || !value");
        return NULL;
    }

    size_t idx = -1ULL;
    for (size_t i = 0; i < frame->ncols; ++i)
        if (!strcmp(frame->colnames[i], colname))
        {
            idx = i;
            break;
        }

    if (idx == -1ULL)
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

int frame_update(dataframe_t *frame, const char *keyname, const void *keyval, const char *colname, void *value)
{
    if (!frame || !colname || !value)
        return 0;

    size_t kidx = -1ULL, vidx = -1ULL;
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

    if (vidx == -1ULL)
        return 0;

    for (size_t i = 0; i < frame->nrows; ++i)
    {
        datablock_t *row = frame->rows[i];
        if (!keyval || !column_cmp_value(row->cols[kidx], keyval))
        {
            column_update(row->cols[vidx], value);
        }
    }


    return 1;
}

