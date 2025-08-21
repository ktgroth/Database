
#ifndef GDB_DATA_
#define GDB_DATA_

#include "id.h"
#include "btree.h"
#include "map.h"

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


#define update3(x, cols, data) \
    updateb(x, cols, data)

#define update4(x, cols, data, id) \
    _Generic((x), \
        dataframe_t *:  updatef, \
        btree_t *:      updatet, \
        hashmap_t *:    updatem  \
    )(x, cols, data, id)

#define update(...) \
    GET_MACRO(__VA_ARGS__, update4, update3)(__VA_ARGS__)

int updateb(datablock_t *block, char *cols, void *data);
int updatef(datablock_t *frame, char *cols, void *data, db_id_t id);
int updatet(btree_t *tree, char *cols, void *data, db_id_t id);
int updatem(hashmap_t *map, char *cols, void *data, db_id_t id);

#endif

