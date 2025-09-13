
#include <stdio.h>
#include <stdlib.h>

#include "include/column.h"


column_t *init_column(const char *name, const datafield_t *field)
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

