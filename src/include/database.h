
#ifndef GDB_DB_
#define GDB_DB_

#include "data.h"
#include "btree.h"
#include "map.h"

#define find(x, id) \
    _Generic((x), \
        dataframe_t *:  findf, \
        btree_t *:      findt, \
        hashmap_t *:    findm  \
    )(x, id)

datablock_t *findf(dataframe_t *frame, db_id_t id);
datablock_t *findt(btree_t *tree, db_id_t id);
datablock_t *findm(hashmap_t *map, db_id_t id);


#define GET_MACRO(_1, _2, _3, _4, NAME, ...) NAME

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

