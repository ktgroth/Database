
#ifndef GDB_DATAFRAME_
#define GDB_DATAFRAME_

#include "datablock.h"


typedef struct
{
    size_t        ncols;
    const char  **colnames;
    const type_e *coltypes;

    size_t        nrows;
    size_t        capacity;
    datablock_t **rows;
} dataframe_t;


datablock_t         *construct_block(const dataframe_t *frame, const void **values);


dataframe_t         *init_frame(size_t ncols, const char **colnames, const type_e *coltypes);
void                 free_frame(dataframe_t *frame);
void                 print_frame(const dataframe_t *frame);


int                  frame_insert(dataframe_t *frame, const void **values);
int                  frame_add(dataframe_t *frame, const datablock_t *block);
int                  frame_remove(dataframe_t *frame, const char *colname, const void *value);
const dataframe_t   *frame_lookup(const dataframe_t *frame, const char *colname, const void *value);
int                  frame_update(dataframe_t *frame, const char *keyname, const void *keyval, const char *colname, const void *value);

#endif

