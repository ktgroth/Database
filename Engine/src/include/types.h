
#ifndef GDB_TYPES_
#define GDB_TYPES_

typedef enum
{
    COL_INT,
    COL_FLOAT,
    COL_STRING,
} type_e;


void print_value(type_e type, const void *value);


int cmp_value(type_e type, const void *a, const void *b);

#endif

