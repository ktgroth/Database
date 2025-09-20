
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/column.h"


column_t *init_column(char *name, datafield_t *field)
{
    column_t *column = (column_t *)calloc(1, sizeof(column_t));
    if (!column)
    {
        perror("column = calloc(1, sizeof(column_t))");
        return NULL;
    }

    column->name = name;
    column->field = field;

    return column;
}

void free_column(column_t *column)
{
    if (!column)
        return;

    free_field(column->field);
    free(column);
}

#define C(x, y, n, dst, src)          \
    case (x): {                      \
        (dst) = calloc((n), sizeof(y)); \
        if (!(dst))                \
            return NULL;           \
        memcpy((dst), (src), (n) * sizeof(y));  \
        break;                     \
    }

void *copy_key_value(type_e type, void *src)
{
    void *dst = NULL;
    if (!src)
        return NULL;

    switch (type)
    {
        case COL_NULL:
            break;

        C(COL_BOOL, int8_t, 1, dst, src);
        C(COL_INT8, int8_t, 1, dst, src);
        C(COL_INT16, int16_t, 1, dst, src);
        C(COL_INT32, int32_t, 1, dst, src);
        C(COL_INT64, int64_t, 1, dst, src);
        C(COL_FLOAT32, float, 1, dst, src);
        C(COL_FLOAT64, double, 1, dst, src);
        C(COL_STRING, char, strlen((const char *)src), dst, src);
        C(COL_DATETIME, char, strlen((const char *)src), dst, src);

        case COL_BLOB:
            blob_t *blob = (blob_t *)malloc(sizeof(blob_t));
            if (!blob)
                return NULL;

            blob->size = ((blob_t *)src)->size;
            blob->data = malloc(blob->size);
            if (!blob->data)
            {
                free(blob);
                return NULL;
            }

            memcpy(blob->data, ((blob_t *)src)->data, blob->size);
            dst = blob;
            break;

        default:
            fprintf(stderr, "[ERROR] Unhandled Type.");
            break;
    }

    return dst;
}

int column_cmp(const column_t *c1, const column_t *c2)
{
    if (!c1 || !c2)
    {
        perror("!c1 || !c2");
        return 0;
    }

    return field_cmp(c1->field, c2->field);
}

int column_cmp_value(const column_t *col, const void *value)
{
    if (!col || !value)
    {
        perror("!col || !value");
        return 0;
    }

    return field_cmp_value(col->field, value);
}

int column_update(column_t *column, void *value)
{
    if (!column || !value)
        return 0;

    return field_update(column->field, value);
}

