
#include <stdio.h>
#include <string.h>

#include "include/types.h"


void print_value(type_e type, const void *value)
{
    switch (type)
    {
    case COL_INT:
        printf("  %-15d  ", *(int *)value);
        break;

    case COL_FLOAT:
        printf("  %-15f  ", *(double *)value);
        break;

    case COL_STRING:
        printf("  %-15s  ", (const char *)value);
        break;

    default:
        printf("??");
    }
}

int cmp_value(type_e type, const void *a, const void *b)
{
    switch (type)
    {
    case COL_INT:
        return (*(int *)a > *(int *)b) * 1 + (*(int *)a < *(int *)b) * -1;

    case COL_FLOAT:
        return (*(double *)a > *(double *)b) * 1 + (*(double *)a < *(double *)b) * -1;

    case COL_STRING:
        return strcmp((char *)a, (char *)b);

    default:
        return 0;
    }
}

