
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/table.h"


table_t *init_table(const char *name, size_t ncols, const char **colnames, const type_e *coltypes, int indexed, const char *pkname, const type_e pktype)
{
    table_t *tbl = (table_t *)calloc(1, sizeof(table_t));
    if (!tbl)
    {
        perror("tbl = calloc(1, sizeof(table_t))");
        return NULL;
    }

    tbl->name = strdup(name);
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
            print_table_btree(node->children[i], type);

        datablock_t *data = node->dptr[i];
        for (size_t j = 0; j < data->ncols; ++j)
            print_value(data->cols[j]->field->type, data->cols[j]->field->value);
        puts("");
    }

    if (!node->is_leaf)
        print_table_btree(node->children[node->nkeys], type);
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

    printf("TABLE: %s\n", tbl->name);
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

    size_t iidx = -1, cidx = -1;
    for (size_t i = 0; i < tbl->nidxs; ++i)
        if (!strcmp(colname, tbl->idxs[i]->pkname))
        {
            iidx = i;
            break;
        }

    for (size_t i = 0; i < tbl->ncols; ++i)
        if (!strcmp(colname, tbl->colnames[i]))
        {
            cidx = i;
            break;
        }

    if (iidx == -1 || cidx == -1)
    {
        fprintf(stderr, "%s has no associated index.", colname);
        return;
    }

    printf("TABLE: %s\tINDEX: %s\n", tbl->name, tbl->idxs[iidx]->pkname);
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
    print_table_btree(tbl->idxs[iidx]->root, tbl->coltypes[cidx]);
}

static const uint32_t K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

static void sha256_transform(uint32_t state[8], const uint8_t block[64])
{
    uint32_t w[64];
    for (int i = 0; i < 16; ++i)
        w[i] = (block[i * 4] << 24) | (block[i * 4 + 1] << 16) | (block[i * 4 + 2] << 8) | (block[i * 4 + 3]);

    for (int t = 16; t < 64; ++t)
    {
        uint32_t s0 = ROTR(w[t - 15], 7) ^ ROTR(w[t - 15], 18) ^ (w[t - 15] >> 3);
        uint32_t s1 = ROTR(w[t - 2], 17) ^ ROTR(w[t - 2], 19) ^ (w[t - 2] >> 10);
        w[t] = w[t - 16] + s0 + w[t - 7] + s1;
    }

    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];

    for (int t = 0; t < 64; ++t)
    {
        uint32_t s1 = ROTR(e, 6) ^ ROTR(e, 11) ^ ROTR(e, 25);
        uint32_t ch = (e & f) ^ ((~e) & g);
        uint32_t temp1 = h + s1 + ch + K[t] + w[t];
        uint32_t s0 = ROTR(a, 2) ^ ROTR(a, 13) ^ ROTR(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = s0 + maj;

        h = g;
        g = f;
        f = e;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
}

static void sha256(const uint8_t *data, size_t len, uint8_t out[32])
{
    uint32_t state[8] = {
        0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
        0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19
    };

    uint64_t bitlen = (uint64_t)len * 8;
    size_t padlen = 64 - ((len + 9) % 64);
    size_t total = len + 1 + padlen + 8;
    uint8_t *buf = malloc(total);

    if (!buf)
        return;

    memcpy(buf, data, len);
    buf[len] = 0x80;
    memset(buf + len + 1, 0, padlen);
    for (int i = 0; i < 8; ++i)
        buf[total - 8 + i] = (uint8_t)(bitlen >> (56 - 8 * i)) & 0xFF;

    for (size_t i = 0; i < total; i += 64)
        sha256_transform(state, buf + i);
    free(buf);

    for (int i = 0; i < 8; ++i)
    {
        out[i * 4 + 0] = (state[i] >> 24) & 0xFF;
        out[i * 4 + 1] = (state[i] >> 16) & 0xFF;
        out[i * 4 + 2] = (state[i] >> 8) & 0xFF;
        out[i * 4 + 3] = (state[i] >> 0) & 0xFF;
    }
}

static void hex_to_ascii(const unsigned char in[32], char out[65])
{
    static const char hex[] = "0123456789abcdef";

    for (int i = 0; i < 32; ++i)
    {
        unsigned char byte = in[i];
        out[i * 2 + 0] = hex[(byte >> 4) & 0x0F];
        out[i * 2 + 1] = hex[(byte >> 0) & 0x0F];
    }

    out[64] = '\0';
}

static const void *table_generate_key(table_t *tbl)
{
    void *ptr = NULL;
    switch (tbl->pktype & ~KEY_PK & ~KEY_FK)
    {
        case COL_BOOL:
        case COL_INT8:
            ptr = (uint8_t *)malloc(sizeof(uint8_t));
            *(uint8_t *)ptr = tbl->indexed++;
            break;

        case COL_INT16:
            ptr = (uint16_t *)malloc(sizeof(uint16_t));
            *(uint16_t *)ptr = tbl->indexed++;
            break;

        case COL_INT32:
            ptr = (uint32_t *)malloc(sizeof(uint32_t));
            *(uint32_t *)ptr = tbl->indexed++;
            break;

        case COL_INT64:
            ptr = (uint64_t *)malloc(sizeof(uint64_t));
            *(uint64_t *)ptr = tbl->indexed++;
            break;

        case COL_FLOAT32:
            ptr = (float *)malloc(sizeof(float *));
            *(float *)ptr = (float)tbl->indexed++;
            break;

        case COL_FLOAT64:
            ptr = (double *)malloc(sizeof(double));
            *(double *)ptr = (double)tbl->indexed++;
            break;

        case COL_STRING:
            unsigned char hex[32];
            char *key = (char *)malloc(65 * sizeof(char));
            if (!key)
                return NULL;

            size_t indexed_value = tbl->indexed++;
            sha256((unsigned char *)&indexed_value, sizeof(size_t), hex);
            hex_to_ascii(hex, key);

            return key;

        default:
            break;
    }

    return ptr;
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

#define MIN(x, y) ((x) > (y) ? y : x) 

static int check_cols(const table_t *tbl, const datablock_t *block)
{
    for (size_t i = 0; i < MIN(tbl->ncols, block->ncols); ++i)
        if (strcmp(tbl->colnames[i], block->cols[i]->name))
            return 0;

    if (tbl->ncols != block->ncols)
        return 0;

    return 1;
}

static column_t **update_column_metadata(const datablock_t *orig, const table_t *tbl, void *key)
{
    size_t ncols = tbl->ncols;
    column_t **new_cols = calloc(ncols, sizeof(column_t *));
    if (!new_cols)
        return NULL;

    if (tbl->indexed)
    {
        datafield_t *pk_field = init_field(tbl->pktype, key);
        if (!pk_field)
        {
            free(new_cols);
            return NULL;
        }

        new_cols[0] = init_column(tbl->pkname, pk_field);
        if (!new_cols[0])
        {
            free_field(pk_field);
            free(new_cols);
            return NULL;
        }
    }

    for (size_t i = tbl->indexed != 0; i < ncols; ++i)
    {
        const char *colname = tbl->colnames[i];

        const column_t *src_col = NULL;
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
            new_field = init_field(tbl->coltypes[i], NULL);

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

static datablock_t *update_block_structure(const datablock_t *orig, const table_t *tbl, void *key)
{
    if (!orig || !tbl)
        return NULL;

    column_t **new_cols = update_column_metadata(orig, tbl, key);
    if (!new_cols)
        return NULL;

    datablock_t *new_block = init_block(tbl->ncols, tbl->colnames, tbl->coltypes, NULL);
    if (!new_block)
    {
        for (size_t i = 0; i < tbl->ncols; ++i)
            free_column(new_cols[i]);
        free(new_cols);
        return NULL;
    }

    new_block->ncols = tbl->ncols;
    new_block->cols = new_cols;

    return new_block;
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

static int table_add_to_index(btree_t *tree, datablock_t *row)
{
    const char *colname = tree->pkname;

    size_t idx = -1;
    for (size_t i = 0; i < row->ncols; ++i)
        if (!strcmp(colname, row->cols[i]->name))
        {
            idx = i;
            break;
        }

    if (idx == -1)
        return 0;

    return btree_add(tree, row->cols[idx]->field->value, row);
}

int table_add(table_t *tbl, datablock_t *row)
{
    if (!tbl || !row)
    {
        fprintf(stderr, "!tbl || !row");
        return 0;
    }

    void *key = NULL;
    if (tbl->indexed)
        key = table_generate_key(tbl);

    if (!check_cols(tbl, row) && !(row = update_block_structure(row, tbl, key)))
        return 0;

    for (size_t i = 0; i < tbl->nidxs; ++i)
        if (!table_add_to_index(tbl->idxs[i], row))
            return 0;

    if (!storage_add(tbl->rows, key, row))
        return 0;

    ++tbl->nrows;
    return 1;
}

static int table_remove_from_index(btree_t *tree, void *value)
{
    return btree_remove_key(tree, value);
}

int table_remove(table_t *tbl, const char *colname, void *value)
{
    if (!tbl || !colname || !value)
    {
        fprintf(stderr, "!tbl || !colname || !value");
        return 0;
    }

    for (size_t i = 0; i < tbl->nidxs; ++i)
        if (!strcmp(tbl->idxs[i]->pkname, colname) && !table_remove_from_index(tbl->idxs[i], value))
            return 0;

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

    for (size_t i = 0; i < tbl->nidxs; ++i)
        if (!strcmp(tbl->idxs[i]->pkname, colname))
            return btree_lookup_key(tbl->idxs[i], value);

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

