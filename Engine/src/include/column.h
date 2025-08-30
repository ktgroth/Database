
#ifndef GDB_COLUMN_
#define GDB_COLUMN_

#include "types.h"


typedef struct
{
    const char  *name;
    type_e      type;
    void        *value;
} column_t;

#endif

