
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#include "include/database.h"


database_t *init_database(const char *name)
{
    database_t *db = (database_t *)calloc(1, sizeof(database_t));
    if (!db)
    {
        perror("db = calloc(1, sizeof(database_t))");
        return NULL;
    }

    db->name = strdup(name);
    db->capacity = 1;
    db->tables = (table_t **)calloc(db->capacity, sizeof(table_t *));
    if (!db->tables)
    {
        free(db);
        return NULL;
    }

    return db;
}

void free_database(database_t *db)
{
    if (!db)
        return;

    free(db->name);
    for (size_t i = 0; i < db->ntables; ++i)
        free_table(db->tables[i]);
    if (db->tables)
        free(db->tables);
}

void print_database(const database_t *db)
{
    if (!db)
        return;

    printf("DATABASE: %s\n", db->name);
    for (size_t i = 0; i < db->ntables; ++i)
    {
        table_t *tbl = db->tables[i];
        printf(" { TABLE: %s }", tbl->name);
        for (size_t j = 0; j < tbl->ncols; ++j)
            printf(" %*s ", (int)strlen(tbl->colnames[j]), tbl->colnames[j]);
        puts("");
    }
}

#define WRITE_OR_FAIL(ptr, size, nmemb, fp) \
    if (fwrite((ptr), (size), (nmemb), (fp)) != (nmemb)) { \
        perror("fwrite"); \
        fclose(fp);       \
        return 0;         \
    }

const size_t sizes[] = {
    0,
    sizeof(int8_t),
    sizeof(int8_t),
    sizeof(int16_t),
    sizeof(int32_t),
    sizeof(int64_t),
    sizeof(float),
    sizeof(double),
    sizeof(char *),
    sizeof(char *),
    sizeof(blob_t *),
};

#define C(type, value, fp) \
    case (type): \
        WRITE_OR_FAIL((value), sizes[(type)], 1, (fp)); \
        break;

static int database_write_field(FILE *fp, const datafield_t *field)
{
    void *value = field->value;
    type_e type = field->type & ~(KEY_PK | KEY_FK);

    switch (type) {
        case COL_NULL:
            break;

        C(COL_BOOL, value, fp);
        C(COL_INT8, value, fp);
        C(COL_INT16, value, fp);
        C(COL_INT32, value, fp);
        C(COL_INT64, value, fp);
        C(COL_FLOAT32, value, fp);
        C(COL_FLOAT64, value, fp);

        case COL_STRING:
        case COL_DATETIME:
            char *str = (char *)value;
            WRITE_OR_FAIL(str, 1, strlen(str) + 1, fp);
            break;

        case COL_BLOB:
            blob_t *blob = (blob_t *)value;
            WRITE_OR_FAIL(&blob->size, sizeof(size_t), 1, fp);
            WRITE_OR_FAIL(blob->data, 1, blob->size, fp);
            break;

        default:
            fprintf(stderr, "[ERROR] Unhandled type.");
            return 0;
    }

    return 1;
}

static int database_write_block(FILE *fp, const datablock_t *block)
{
    for (size_t i = 0; i < block->ncols; ++i)
        if (!database_write_field(fp, block->cols[i]->field))
            return 0;

    return 1;
}

static int database_write_btree_node(FILE *fp, const btree_node_t *node, const type_e type)
{
    WRITE_OR_FAIL(&node->is_leaf, sizeof(int), 1, fp);
    WRITE_OR_FAIL(&node->nkeys, sizeof(size_t), 1, fp);

    for (size_t i = 0; i < node->nkeys; ++i) {
        if (!database_write_field(fp, init_field(type, node->keys[i])))
            return 0;
        if (!database_write_block(fp, node->dptr[i]))
            return 0;
    }

    if (!node->is_leaf)
        for (size_t i = 0; i <= node->nkeys; ++i)
            if (!database_write_btree_node(fp, node->children[i], type))
                return 0;

    return 1;
}

static int database_write_btree(FILE *fp, const btree_t *tree)
{
    WRITE_OR_FAIL(&tree->u, sizeof(size_t), 1, fp);
    WRITE_OR_FAIL(&tree->ncols, sizeof(size_t), 1, fp);

    if (!database_write_btree_node(fp, tree->root, tree->pktype & ~KEY_PK & ~KEY_FK))
        return 0;

    return 1;
}

static int database_write_frame(FILE *fp, const dataframe_t *frame)
{
    for (size_t i = 0; i < frame->nrows; ++i)
        if (!database_write_block(fp, frame->rows[i]))
            return 0;

    return 1;
}

static int database_write_table(const char *filepath, const table_t *tbl)
{
    FILE *fp = fopen(filepath, "wb");
    if (!fp)
    {
        perror("fopen");
        return 0;
    }

    WRITE_OR_FAIL("TABLE", 1, sizeof("TABLE"), fp);

    WRITE_OR_FAIL(tbl->name, 1, strlen(tbl->name) + 1, fp);
    WRITE_OR_FAIL(&tbl->ncols, sizeof(size_t), 1, fp);
    WRITE_OR_FAIL(&tbl->nrows, sizeof(size_t), 1, fp);

    for (size_t i = 0; i < tbl->ncols; ++i)
    {
        WRITE_OR_FAIL(tbl->colnames[i], 1, strlen(tbl->colnames[i]) + 1, fp);
        WRITE_OR_FAIL(&tbl->coltypes[i], sizeof(type_e), 1, fp);
    }

    if (tbl->indexed)
    {
        if (!database_write_btree(fp, tbl->rows->btree))
            return 0;
    } else {
        if (!database_write_frame(fp, tbl->rows->frame))
            return 0;
    }

    fclose(fp);
    return 1;
}

int database_write(database_t *db)
{
    if (mkdir(db->name, 0777) == -1 && errno != EEXIST)
    {
        perror("mkdir");
        return 0;
    }

    for (size_t i = 0; i < db->ntables; ++i)
    {
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s.dat", db->name, db->tables[i]->name);

        database_write_table(filepath, db->tables[i]);
    }

    return 1;
}

database_t *database_read_path(const char *filepath)
{
    FILE *fp = fopen(filepath, "rb");
    if (!fp)
        return NULL;

    return database_read(fp);
}

database_t *database_read(FILE *fp)
{
    if (!fp)
        return NULL;

    return NULL;
}

int database_add_table(database_t *db, table_t *tbl)
{
    if (!db || !tbl)
    {
        fprintf(stderr, "!db || !tbl");
        return 0;
    }

    if (db->ntables == db->capacity)
    {
        table_t **new_tables = (table_t **)realloc(db->tables, (db->capacity * 3 + 1) * sizeof(table_t *));
        if (!new_tables)
            return 0;

        db->capacity = (db->capacity * 3) + 1; 
        db->tables = new_tables;
    }

    db->tables[db->ntables++] = tbl;

    return 1;
}

