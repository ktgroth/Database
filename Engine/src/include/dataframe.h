
#ifndef GDB_DATAFRAME_
#define GDB_DATAFRAME_

#include <stddef.h>

#include "datablock.h"


typedef struct
{
    size_t        ncols;
    char  **colnames;
    type_e *coltypes;

    size_t        nrows;
    size_t        capacity;
    datablock_t **rows;
} dataframe_t;


dataframe_t         *init_frame(size_t ncols, char **colnames, type_e *coltypes);
void                 free_frame(dataframe_t *frame);
void                 print_frame(const dataframe_t *frame);


int                  frame_insert(dataframe_t *frame, void **values);
int                  frame_add(dataframe_t *frame, datablock_t *block);
int                  frame_remove(dataframe_t *frame, const char *colname, void *value);
const dataframe_t   *frame_lookup(const dataframe_t *frame, const char *colname, void *value);
int                  frame_update(dataframe_t *frame, const char *keyname, const void *keyval, const char *colname, void *value);

#endif

