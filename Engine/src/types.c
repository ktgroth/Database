
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/types.h"


void print_value(type_e type, const void *value)
{
    switch (type)
    {
        case COL_NULL:
            printf(" %-15s ", "NULL");
            break;

        case COL_BOOL:
        case COL_INT8:
            printf(" %-15d ", *(int8_t *)value);
            break;
        case COL_INT16:
            printf(" %-15d ", *(int16_t *)value);
            break;
        case COL_INT32:
            printf(" %-15d ", *(int32_t *)value);
            break;
        case COL_INT64:
            printf(" %-15ld ", *(int64_t *)value);
            break;

        case COL_FLOAT32:
            printf(" %-15f ", *(float *)value);
            break;
        case COL_FLOAT64:
            printf(" %-15f ", *(double *)value);
            break;

        case COL_STRING:
        case COL_DATETIME:
            printf(" %-15s ", (const char *)value);
            break;

        case COL_BLOB:
            printf(" %-15p ", value);
            break;
    }
}

int value_cmp(type_e type, const void *a, const void *b)
{
    switch (type)
    {
        case COL_NULL:
            return 0;

        case COL_BOOL:
        case COL_INT8:
            return *(int8_t *)a - *(int8_t *)b;

        case COL_INT16:
            return *(int16_t *)a - *(int16_t *)b;

        case COL_INT32:
            return *(int32_t *)a - *(int32_t *)b;

        case COL_INT64:
            return *(int64_t *)a - *(int64_t *)b;

        case COL_FLOAT32:
            return *(float *)a - *(float *)b;

        case COL_FLOAT64:
            return *(double *)a - *(double *)b;

        case COL_STRING:
            return strcmp((const char *)a, (const char *)b);

        default:
            return 1;
    }
}


datafield_t *init_field(type_e type, const void *value)
{
    datafield_t *field = (datafield_t *)calloc(1, sizeof(datafield_t));
    if (!field)
    {
        perror("field = calloc(1, sizeof(datafield_t))");
        return NULL;
    }

    field->type = type;
    switch (type)
    {
        case COL_NULL:
            break;

        case COL_BOOL:
        case COL_INT8:
        case COL_INT16:
        case COL_INT32:
        case COL_INT64:
        case COL_FLOAT32:
        case COL_FLOAT64:
        case COL_STRING:
        case COL_DATETIME:
            field->value = value;
            break;

        case COL_BLOB:
            field->blob = construct_blob_path(value);
            break;
    }

    return field;
}

void free_field(datafield_t *field)
{
    if (!field)
        return;

    if (field->type == COL_BLOB)
        free_blob(field->blob);
    else
        free(field->value);
    free(field);
}

int field_cmp(const datafield_t *f1, const datafield_t *f2)
{
    if (!f1 || !f2 || f1->type != f2->type)
    {
        perror("!f1 || !f2 || f1->type != f2->type");
        return 0;
    }

    return value_cmp(f1->type, f1->value, f2->value);
}

int field_cmp_value(const datafield_t *field, const void *value)
{
    if (!field || !value)
    {
        perror("!field || !value");
        return 0;
    }

    return value_cmp(field->type, field->value, value);
}

int field_update(datafield_t *field, const void *value)
{
    if (!field || !value)
        return 0;

    switch (field->type)
    {
        case COL_NULL:
            break;

        case COL_BOOL:
        case COL_INT8:
        case COL_INT16:
        case COL_INT32:
        case COL_INT64:
        case COL_FLOAT32:
        case COL_FLOAT64:
        case COL_STRING:
        case COL_DATETIME:
            field->value = value;
            break;

        case COL_BLOB:
            field->blob = construct_blob_path(value);
            break;

    }

    return 1;
}

blob_t *construct_blob_path(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (!fp)
    {
        perror("fp = fopen(path)");
        return NULL;
    }

    return construct_blob_file(fp);
}

blob_t *construct_blob_file(FILE *fp)
{
    if (!fp)
        return NULL;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    blob_t *blob = (blob_t *)calloc(1, sizeof(blob_t));
    if (!blob)
    {
        perror("blob = calloc(1, sizeof(blob_t))");
        fclose(fp);
        return NULL;
    }

    blob->size = size;
    blob->data = (char *)malloc(size);
    if (!blob->data)
    {
        free(blob);
        fclose(fp);
        return NULL;
    }

    fread(blob->data, 1, size, fp);
    fclose(fp);
    return blob;
}

void free_blob(blob_t *blob)
{
    if (!blob)
        return;

    free(blob->data);
    free(blob);
}

