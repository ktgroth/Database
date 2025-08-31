
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include "include/table.h"

#define BRANCHING_FACTOR 80


table_t *init_table(size_t ncols, const char **colnames, const type_e *coltypes, int indexed)
{
    table_t *tbl = (table_t *)calloc(1, sizeof(table_t));
    if (!tbl)
    {
        perror("tbl = calloc(1, sizeof(table_t))");
        return NULL;
    }

    tbl->ncols = ncols;
    tbl->nindexes = (indexed != 0);

    tbl->colnames = (char **)calloc(ncols, sizeof(const char *));
    tbl->coltypes = (type_e *)calloc(ncols, sizeof(type_e));
    if (!tbl->colnames || !tbl->coltypes)
    {
        perror("tbl->colnames = realloc(ncols, sizeof(const char *))");
        perror("tbl->coltypes = realloc(ncols, sizeof(type_e))");
        free(tbl);
        return NULL;
    }

    for (size_t i = 0; i < ncols; ++i)
    {
        tbl->colnames[i] = strdup(colnames[i]);
        tbl->coltypes[i] = coltypes[i];
    }

    if (indexed)
    {
        tbl->indexes = (btree_t **)calloc(1, sizeof(btree_t *));
        if (!tbl->indexes)
        {
            perror("tbl->indexes = calloc(nindexes, sizeof(btree_t *))");
            free_table(tbl);
            return NULL;
        }
        tbl->nindexes = 1;

        char  **new_names = realloc(tbl->colnames, (ncols + 1) * sizeof(char *));
        type_e *new_types = realloc(tbl->coltypes, (ncols + 1) * sizeof(type_e));
        if (!new_names || !new_types)
        {
            perror("new_names = calloc(ncols + 1, sizeof(const char *))");
            perror("new_types = calloc(ncols + 1, sizeof(type_e))");
            free_table(tbl);
            return NULL;
        }

        ++tbl->ncols;
        tbl->colnames = new_names;
        tbl->coltypes = new_types;
        for (size_t i = 0; i < ncols; ++i)
        {
            tbl->colnames[i + 1] = tbl->colnames[i];
            tbl->coltypes[i + 1] = tbl->coltypes[i];
        }

        tbl->colnames[0] = "ID";
        tbl->coltypes[0] = COL_INT;
        tbl->indexes[0] = init_btree(BRANCHING_FACTOR, tbl->colnames[0], tbl->coltypes[0]);
    }

    tbl->frame = init_frame(tbl->ncols, (const char **)tbl->colnames, tbl->coltypes);
    if (!tbl->frame)
    {
        if (tbl->nindexes)
        {
            free_btree(tbl->indexes[0]);
            free(tbl->indexes);
        }

        for (size_t i = 0; i < ncols; ++i)
            free(tbl->colnames[i]);
        free(tbl->colnames);
        free(tbl->coltypes);
        free(tbl);
        return NULL;
    }

    return tbl;
}

void free_table(table_t *tbl)
{
    if (!tbl)
        return;

    for (size_t i = 0; i < tbl->nindexes; ++i)
        free_btree(tbl->indexes[i]);
    free(tbl->indexes);
    free_frame(tbl->frame);

    for (size_t i = 0; i < tbl->ncols; ++i)
        free(tbl->colnames[i]);
    free(tbl->colnames);
    free(tbl->coltypes);
    free(tbl);
}

int is_index(const table_t *tbl, const char *colname)
{
    for (size_t i = 0; i < tbl->nindexes; ++i)
        if (!strcmp(tbl->indexes[i]->colname, colname))
        {
            return 1;
        }

    return 0;
}

void print_table(const table_t *tbl)
{
    if (!tbl)
        return;

    for (size_t i = 0; i < tbl->ncols; ++i)
    {
        int isidx = is_index(tbl, tbl->colnames[i]);
        if (isidx)
            printf("\x1b[1m");

        printf("  %-15s  ", tbl->colnames[i]);
        if (isidx)
            printf("\x1b[0m");
    }
    puts("");

    for (size_t i = 0; i < tbl->nrows; ++i)
    {
        datablock_t *row = tbl->frame->rows[i];
        for (size_t j = 0; j < row->ncols; ++j)
        {
            print_value(row->cols[j].type, row->cols[j].value);
        }
        puts("");
    }
}

int table_add(table_t *tbl, datablock_t *block)
{
    if (!tbl || !block)
        return 0;

    if (!frame_add(tbl->frame, block))
        return 0;

    for (size_t i = 0; i < tbl->nindexes; ++i)
    {
        size_t idx = SIZE_MAX;
        for (size_t j = 0; j < tbl->ncols; ++j)
            if (!strcmp(tbl->colnames[j], tbl->indexes[i]->colname))
            {
                idx = j;
                break;
            }

        btree_add(tbl->indexes[i], block->colnames[j], block);
    }

    return 1;
}

const dataframe_t *table_search(const table_t *tbl, size_t ncols, const char **cols, const void **values)
{

}

int table_update(table_t *tbl, const char *keycol, const void *keyval, const char *col, void *value)
{

}

int table_delete(table_t *tbl, const char *keycol, const void *keyval)
{

}

