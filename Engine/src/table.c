
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/table.h"


table_t *init_table(size_t ncols, const char **colnames, const type_e *coltypes, int indexed, const char *pkname, const type_e pktype)
{
    table_t *tbl = (table_t *)calloc(1, sizeof(table_t));
    if (!tbl)
    {
        perror("tbl = calloc(1, sizeof(table_t))");
        return NULL;
    }

    tbl->ncols = ncols;
    tbl->colnames = (const char **)calloc(ncols, sizeof(const char *));
    tbl->coltypes = (const type_e *)calloc(ncols, sizeof(const type_e));

    type_e *cts = tbl->coltypes;
    for (size_t i = 0; i < ncols; ++i)
    {
        tbl->colnames[i] = strdup(colnames[i]);
        cts[i] = coltypes[i];
    }

    if (indexed)
    {
        tbl->indexed = 1;
        tbl->pkname = strdup(pkname);
        tbl->pktype = pktype;
        tbl->rows = init_storage(STORAGE_BTREE, ncols, tbl->colnames, tbl->coltypes, pkname, pktype);
    } else
        tbl->rows = init_storage(STORAGE_FRAME, ncols, tbl->colnames, tbl->coltypes, NULL, COL_NULL);


    return tbl;
}

void free_table(table_t *tbl)
{
    if (!tbl)
        return;

    free(tbl->name);

    for (size_t i = 0; i < tbl->nidxs; ++i)
        free_btree(tbl->idxs[i]);
    free(tbl->idxs);

    for (size_t i = 0; i < tbl->ncols; ++i)
        free(tbl->colnames[i]);
    free(tbl->colnames);
    free(tbl->coltypes);

    if (tbl->indexed)
        free(tbl->pkname);

    free_storage(tbl->rows);
    free(tbl);
}

int is_index(const table_t *tbl, const char *colname)
{
    if (tbl->indexed && !strcmp(colname, tbl->pkname))
         return 1;

    for (size_t i = 0; i < tbl->nidxs; ++i)
    {
        if (!strcmp(colname, tbl->idxs[i]->pkname))
            return 1;
    }

    return 0;
}

void print_table_btree(btree_node_t *node, type_e type)
{
    if (!node)
        return;

    for (size_t i = 0; i < node->nkeys; ++i)
    {
        if (!node->is_leaf)
            print_btree_node(node->children[i], type);

        print_value(type, node->keys[i]);
        datablock_t *data = node->dptr[i];
        for (size_t j = 0; j < data->ncols; ++j)
            print_value(data->cols[j]->field->type, data->cols[j]->field->value);
        puts("");
    }

    if (!node->is_leaf)
        print_btree_node(node->children[node->nkeys], type);
}

void print_table_frame(dataframe_t *frame)
{
    for (size_t i = 0; i < frame->nrows; ++i)
    {
        datablock_t *row = frame->rows[i];
        for (size_t j = 0; j < row->ncols; ++j)
            print_value(row->cols[j]->field->type, row->cols[j]->field->value);
        puts("");
    }
}

void print_table(const table_t *tbl)
{
    if (!tbl)
        return;

    if (tbl->indexed)
    {
        printf("\033[33m\033[1m");
        printf(" %-15s ", tbl->pkname);
        printf("\033[0m");
    }

    for (size_t i = 0; i < tbl->ncols; ++i)
    {
        int idx = is_index(tbl, tbl->colnames[i]);
        if (idx)
            printf("\033[32m\033[1m");
        printf(" %-15s\033[0m ", tbl->colnames[i]);
    }
    puts("");

    if (tbl->indexed)
        print_table_btree(tbl->rows->btree->root, tbl->pktype);
    else
        print_table_frame(tbl->rows->frame);
}

const void *table_generate_key(table_t *tbl)
{
    switch (tbl->pktype)
    {
        case COL_BOOL:
        case COL_INT8:
        case COL_INT16:
        case COL_INT32:
        case COL_INT64:
        case COL_FLOAT32:
        case COL_FLOAT64:
            size_t *ptr = (size_t *)malloc(sizeof(size_t));
            *ptr = tbl->indexed++;
            return ptr;

        default:
            return 0;
    }
}

int table_insert(table_t *tbl, void **values)
{
    if (!tbl || !values)
    {
        perror("!tbl || !values");
        return 0;
    }

    const void *key = NULL;
    if (tbl->indexed)
        key = table_generate_key(tbl);

    if (storage_insert(tbl->rows, key, values))
    {
        ++tbl->nrows;
        return 1;
    }

    return 0;
}

int table_add(table_t *tbl, datablock_t *row)
{
    if (!tbl || !row)
    {
        perror("!tbl || !row");
        return 0;
    }

    const void *key = NULL;
    if (tbl->indexed)
        key = table_generate_key(tbl);

    if (storage_add(tbl->rows, key, row))
    {
        ++tbl->nrows;
        return 1;
    }

    return 0;
}

int table_remove(table_t *tbl, const char *colname, void *value)
{
    if (!tbl || !colname || !value)
    {
        perror("!tbl || !colname || !value");
        return 0;
    }

    if (storage_remove(tbl->rows, colname, value))
    {
        --tbl->nrows;
        return 1;
    }

    return 0;
}

const dataframe_t *table_lookup(const table_t *tbl, const char *colname, void *value)
{
    if (!tbl || !colname || !value)
    {
        perror("!tbl || !colname || !value");
        return NULL;
    }

    return storage_lookup(tbl->rows, colname, value);
}

int table_update(table_t *tbl, const char *keyname, const void *keyval, const char *colname, void *value)
{
    if (!tbl || !keyname || !keyval || !colname || !value)
    {
        perror("!tbl || !keyname || !keyval || !colname || !value");
        return 0;
    }

    return storage_update(tbl->rows, keyname, keyval, colname, value);
}

