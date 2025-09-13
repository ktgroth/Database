
#ifndef GDB_DATABASE_
#define GDB_DATABASE_

#include "table.h"


typedef struct
{
    const char  *name;

    size_t       ntables;
    table_t     *tables;
} database_t;

#endif

