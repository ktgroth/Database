
#ifndef GDB_DATA_
#define GDB_DATA_

#include "id.h"

typedef struct
{
    size_t nocls;
    char *cols;
    char *colt;
    void *data;
} datablock_t;

typedef struct
{
    size_t ncols;
    char *cols;
    char *colt;
    void **data;
} dataframe_t;


datablock_t *init_block(size_t ncols, char *cols, char *colt, void *data);
dataframe_t *init_frame(size_t ncols, char *cols, char *colt, void **data);

#endif

