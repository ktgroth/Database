#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include "include/table.h"
#include "include/datablock.h"
#include "include/btree.h"

#define BRANCHING_FACTOR 80

/* -------------------------------------------------------------------------- */
/* Helper functions                                                          */
/* -------------------------------------------------------------------------- */

static int is_index(const table_t *tbl, const char *colname)
{
    if (!tbl || !colname)
        return 0;

    for (size_t i = 0; i < tbl->nindexes; ++i)
    {
        if (!strcmp(tbl->indexes[i]->colname, colname))
            return 1;
    }
    return 0;
}

/* -------------------------------------------------------------------------- */
/* Table creation and destruction                                            */
/* -------------------------------------------------------------------------- */

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
        perror("tbl->colnames = calloc(ncols, sizeof(const char *))");
        perror("tbl->coltypes = calloc(ncols, sizeof(type_e))");
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

        /* Add an implicit ID column at the front */
        char **new_names = realloc(tbl->colnames, (ncols + 1) * sizeof(char *));
        type_e *new_types = realloc(tbl->coltypes, (ncols + 1) * sizeof(type_e));
        if (!new_names || !new_types)
        {
            perror("new_names = realloc(ncols + 1, sizeof(const char *))");
            perror("new_types = realloc(ncols + 1, sizeof(type_e))");
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

/* -------------------------------------------------------------------------- */
/* Printing                                                                 */
/* -------------------------------------------------------------------------- */

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

    for (size_t i = 0; i < tbl->frame->nrows; ++i)
    {
        datablock_t *row = tbl->frame->rows[i];
        for (size_t j = 0; j < row->ncols; ++j)
        {
            print_value(row->cols[j].type, row->cols[j].value);
        }
        puts("");
    }
}

/* -------------------------------------------------------------------------- */
/* Table operations                                                         */
/* -------------------------------------------------------------------------- */

int table_add(table_t *tbl, datablock_t *block)
{
    if (!tbl || !block)
        return 0;

    /* Add to the dataframe first */
    if (!frame_add(tbl->frame, block))
        return 0;

    /* If the table is indexed, insert into the B‑tree */
    if (tbl->indexed)
    {
        /* The key is the first column (ID) */
        void *key = block->cols[0].value;
        if (!btree_add(tbl->indexes[0], key, block))
            return 0;
    }

    return 1;
}

const dataframe_t *table_search(const table_t *tbl, size_t ncols, const char **cols, const void **values)
{
    if (!tbl || !cols || !values)
        return NULL;

    /* Linear scan – sufficient for a small demo implementation */
    for (size_t i = 0; i < tbl->frame->nrows; ++i)
    {
        datablock_t *row = tbl->frame->rows[i];
        int match = 1;
        for (size_t j = 0; j < ncols; ++j)
        {
            /* Find the column index in the table */
            size_t col_idx = 0;
            for (; col_idx < tbl->ncols; ++col_idx)
                if (!strcmp(tbl->colnames[col_idx], cols[j]))
                    break;
            if (col_idx == tbl->ncols)
            {
                match = 0;
                break;
            }

            /* Compare values */
            if (cmp_value(tbl->coltypes[col_idx], row->cols[col_idx].value, values[j]) != 0)
            {
                match = 0;
                break;
            }
        }
        if (match)
            return tbl->frame;
    }

    return NULL;
}

int table_update(table_t *tbl, const char *keycol, const void *keyval, const char *col, void *value)
{
    if (!tbl || !keycol || !keyval || !col)
        return 0;

    /* Find the row by keycol/keyval */
    size_t key_idx = 0;
    for (; key_idx < tbl->ncols; ++key_idx)
        if (!strcmp(tbl->colnames[key_idx], keycol))
            break;
    if (key_idx == tbl->ncols)
        return 0;

    size_t col_idx = 0;
    for (; col_idx < tbl->ncols; ++col_idx)
        if (!strcmp(tbl->colnames[col_idx], col))
            break;
    if (col_idx == tbl->ncols)
        return 0;

    /* Linear scan to find the matching row */
    for (size_t i = 0; i < tbl->frame->nrows; ++i)
    {
        datablock_t *row = tbl->frame->rows[i];
        if (cmp_value(tbl->coltypes[key_idx], row->cols[key_idx].value, keyval) == 0)
        {
            /* Update the column */
            if (!frame_update(tbl->frame, keycol, keyval, col, value))
                return 0;

            /* If the table is indexed and the key column was updated,
               we need to update the B‑tree entry as well. */
            if (tbl->indexed && key_idx == 0)
            {
                /* Remove old entry */
                btree_remove(tbl->indexes[0], keyval);
                /* Insert new entry */
                btree_add(tbl->indexes[0], value, row);
            }
            return 1;
        }
    }

    return 0;
}

int table_delete(table_t *tbl, const char *keycol, const void *keyval)
{
    if (!tbl || !keycol || !keyval)
        return 0;

    /* Find the key column index */
    size_t key_idx = 0;
    for (; key_idx < tbl->ncols; ++key_idx)
        if (!strcmp(tbl->colnames[key_idx], keycol))
            break;
    if (key_idx == tbl->ncols)
        return 0;

    /* Linear scan to find the matching row */
    for (size_t i = 0; i < tbl->frame->nrows; ++i)
    {
        datablock_t *row = tbl->frame->rows[i];
        if (cmp_value(tbl->coltypes[key_idx], row->cols[key_idx].value, keyval) == 0)
        {
            /* Remove from the dataframe */
            if (!frame_remove(tbl->frame, keycol, keyval))
                return 0;

            /* If indexed, remove from B‑tree */
            if (tbl->indexed && key_idx == 0)
                btree_remove(tbl->indexes[0], keyval);

            return 1;
        }
    }

    return 0;
}
