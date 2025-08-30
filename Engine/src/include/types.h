
#ifndef GDB_TYPES_
#define GDB_TYPES_

typedef enum
{
    COL_INT,
    COL_FLOAT,
    COL_STRING,
} type_e;


void print_value(type_e type, const void *value);


#define LT(type, a, b)  cmp_value(type, a, b) == -1
#define LTE(type, a, b) cmp_value(type, a, b) <= 0
#define EQ(type, a, b)  cmp_value(type, a, b) == 0
#define NE(type, a, b)  cmp_value(type, a, b) != 0
#define GTE(type, a, b) cmp_value(type, a, b) >= 0
#define GT(type, a, b)  cmp_value(type, a, b) == 1

int cmp_value(type_e type, const void *a, const void *b);

#endif

