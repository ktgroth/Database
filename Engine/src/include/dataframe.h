
#ifndef GDB_DATAFRAME_
#define GDB_DATAFRAME_

#include "types.h"
#include "datablock.h"

typedef struct dataframe
{
    size_t          ncols;
    size_t          nrows;
    const char      **colnames;
    const type_e    *coltypes;
    datablock_t     **rows;
} dataframe_t;


dataframe_t *init_frame(size_t ncols, const char **colnames, const type_e *coltypes);
void free_frame(dataframe_t *frame);
void print_frame(const dataframe_t *frame);


int frame_add(dataframe_t *frame, const datablock_t *values);
const dataframe_t *frame_find(const dataframe_t *frame, const char *keycol, const void *keyval);
int frame_update(dataframe_t *frame, const char *keycol, const void *keyval, const char *col, void *new_value);
int frame_remove(dataframe_t *frame, const char *keycol, const void *keyval);

#endif

