
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
        int contained = 0;
        for (size_t i = 0; i < ncols; ++i)
        {
            if (!strcmp(pkname, colnames[i]))
            {
                contained = 1;
                break;
            }
        }

        if (!contained)
        {
            const char **new_colnames = realloc(tbl->colnames, (tbl->ncols + 1) * sizeof(const char *));
            const type_e *new_coltypes = realloc(tbl->coltypes, (tbl->ncols + 1) * sizeof(const type_e));
            if (!new_colnames || !new_coltypes)
            {
                free_table(tbl);
                return NULL;
            }

            tbl->colnames = new_colnames;
            tbl->coltypes = new_coltypes;
            cts = tbl->coltypes;

            for (size_t i = tbl->ncols; i > 0; --i)
            {
                tbl->colnames[i] = tbl->colnames[i - 1];
                cts[i] = tbl->coltypes[i - 1];
            }

            ++tbl->ncols;
            tbl->colnames[0] = strdup(pkname);
            cts[0] = pktype;
        }

        tbl->indexed = 1;
        tbl->pkname = strdup(pkname);
        tbl->pktype = pktype;

        tbl->rows = init_storage(STORAGE_BTREE, tbl->ncols, tbl->colnames, tbl->coltypes, strdup(pkname), pktype);
    } else
        tbl->rows = init_storage(STORAGE_FRAME, tbl->ncols, tbl->colnames, tbl->coltypes, NULL, COL_NULL);

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

    for (size_t i = 0; i < tbl->ncols; ++i)
    {
        if (tbl->pkname && !strcmp(tbl->colnames[i], tbl->pkname))
            printf(" \033[33m\033[1m%-15s\033[0m ", tbl->pkname);
        else
        {
            int idx = is_index(tbl, tbl->colnames[i]);
            if (idx)
                printf("\033[32m\033[1m");
            printf(" %-15s\033[0m ", tbl->colnames[i]);
        }
    }
    puts("");

    if (tbl->indexed)
        print_table_btree(tbl->rows->btree->root, tbl->pktype);
    else
        print_table_frame(tbl->rows->frame);
}

void print_table_index(const table_t *tbl, const char *colname)
{
    if (!tbl || !colname)
    {
        fprintf(stderr, "!tbl || !colname");
        return;
    }

    size_t idx = -1;
    for (size_t i = 0; i < tbl->nidxs; ++i)
        if (!strcmp(colname, tbl->idxs[i]->pkname))
        {
            idx = i;
            break;
        }

    if (idx == -1)
    {
        fprintf(stderr, "%s has no associated index.", colname);
        return;
    }

    print_btree(tbl->idxs[idx]);
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

int create_index_btree(btree_t *index, btree_node_t *node, size_t colidx)
{
    if (!index || !node)
        return 0;

    for (size_t i = 0; i < node->nkeys; ++i)
    {
        if (!node->is_leaf)
            create_index_btree(index, node->children[i], colidx);

        datablock_t *row = node->dptr[i];
        if (!btree_add(index, row->cols[colidx]->field->value, row))
            return 0;
    }

    if (!node->is_leaf)
        create_index_btree(index, node->children[node->nkeys], colidx);

    return 1;
}

int create_index_frame(btree_t *index, dataframe_t *frame, size_t colidx)
{
    if (!index || !frame)
        return 0;

    for (size_t i = 0; i < frame->nrows; ++i)
    {
        datablock_t *row = frame->rows[i];
        if (!btree_add(index, row->cols[colidx]->field->value, row))
            return 0;
    }

    return 1;
}

int table_add_index(table_t *tbl, const char *colname)
{
    if (!tbl || !colname)
    {
        fprintf(stderr, "!tbl || !colname");
        return 0;
    }

    size_t idx = -1;
    for (size_t i = 0; i < tbl->ncols; ++i)
        if (!strcmp(colname, tbl->colnames[i]))
        {
            idx = i;
            break;
        }

    if (idx == -1)
        return 0;

    btree_t *index = init_btree(80, tbl->ncols, tbl->colnames, tbl->coltypes, tbl->colnames[idx], tbl->coltypes[idx]);
    btree_t **new_idxs = realloc(tbl->idxs, (tbl->nidxs + 1) * sizeof(btree_t *));
    if (!new_idxs)
    {
        free_btree(index);
        return 0;
    }

    tbl->idxs = new_idxs;
    if (tbl->indexed)
    {
        if (!create_index_btree(index, tbl->rows->btree->root, idx))
        {
            fprintf(stderr, "Could Not Create Index");
            return 0;
        }
    } else
    {
        if (!create_index_frame(index, tbl->rows->frame, idx))
        {
            fprintf(stderr, "Could Not Create Index");
            return 0;
        }
    }

    tbl->idxs[tbl->nidxs++] = index;
    return 1;
}

int table_remove_index(table_t *tbl, const char *colname)
{
    if (!tbl || !colname)
    {
        fprintf(stderr, "!tbl || !colname");
        return 0;
    }

    size_t idx = -1;
    for (size_t i = 0; i < tbl->nidxs; ++i)
        if (!strcmp(colname, tbl->idxs[i]->pkname))
        {
            idx = i;
            break;
        }

    if (idx == -1)
    {
        fprintf(stderr, "%s has no associated index.", colname);
        return 0;
    }

    free_btree(tbl->idxs[idx]);
    for (size_t i = idx; i + 1 < tbl->nidxs; ++i)
        tbl->idxs[i] = tbl->idxs[i + 1];
    --tbl->nidxs;
}

int table_insert(table_t *tbl, void **values)
{
    if (!tbl || !values)
    {
        fprintf(stderr, "!tbl || !values");
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
        fprintf(stderr, "!tbl || !row");
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
        fprintf(stderr, "!tbl || !colname || !value");
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
        fprintf(stderr, "!tbl || !colname || !value");
        return NULL;
    }

    return storage_lookup(tbl->rows, colname, value);
}

int table_update(table_t *tbl, const char *keyname, const void *keyval, const char *colname, void *value)
{
    if (!tbl || !keyname || !keyval || !colname || !value)
    {
        fprintf(stderr, "!tbl || !keyname || !keyval || !colname || !value");
        return 0;
    }

    return storage_update(tbl->rows, keyname, keyval, colname, value);
}

