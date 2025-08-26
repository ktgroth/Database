
#ifndef GDB_DATA_
#define GDB_DATA_

#include "id.h"

typedef struct
{
    size_t ncols;
    char **cols;
    char **colt;
    void **data;
} datablock_t;

typedef struct
{
    size_t ncols;
    size_t nentries;
    char **cols;
    char **colt;
    void ***data;
} dataframe_t;


datablock_t *init_block(size_t ncols, char **cols, char **colt, void **data);
void free_block(datablock_t *block);
void print_block(datablock_t *block);

dataframe_t *init_frame(size_t ncols, char **cols, char **colt);
void free_frame(dataframe_t *frame);
void print_frame(dataframe_t *frame);


void block_change(datablock_t *block, datablock_t *data);

void frame_add(dataframe_t *frame, datablock_t *data);
void frame_remove(dataframe_t *frame, size_t row);
void frame_change(dataframe_t *frame, size_t row, datablock_t *data);
datablock_t *frame_search(dataframe_t *frame, size_t row);

#endif

