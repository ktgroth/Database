
#ifndef GDB_COLUMN_
#define GDB_COLUMN_

#include "types.h"


typedef struct
{
    char  *name;
    datafield_t *field;
} column_t;


column_t    *init_column(const char *name, datafield_t *field);
void         free_column(column_t *column);


void        *copy_key_value(type_e type, void *src);


int          column_cmp(const column_t *c1, const column_t *c2);
int          column_cmp_value(const column_t *col, const void *value);
int          column_update(column_t *column, void *value);

#endif

