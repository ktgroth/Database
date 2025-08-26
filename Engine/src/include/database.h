
#ifndef GDB_DB_
#define GDB_DB_

#include "data.h"
#include "btree.h"
#include "map.h"


#define generate_key(x) \
    _Generic((x), \
        btree_t *:      btree_key, \
        hashmap_t *:    map_key    \
    )(x)

#define add_db(x, data) \
    _Generic((x), \
        dataframe_t *:  frame_add, \
        btree_t *:      btree_add, \
        hashmap_t *:    map_add   \
    )(x, data)


#define find_db(x, id) \
    _Generic((x), \
        dataframe_t *:  frame_search, \
        btree_t *:      btree_search, \
        hashmap_t *:    map_search    \
    )(x, id)


#define change_db(x, id, data) \
    _Generic((x), \
        datablock_t *:  block_change, \
        dataframe_t *:  frame_change, \
        btree_t *:      btree_change, \
        hashmap_t *:    map_change    \
    )(x, id, data)


#define remove_db(x, id) \
    _Generic((x), \
        dataframe_t *:  frame_remove, \
        btree_t *:      btree_remove, \
        hashmap_t *:    map_remove    \
    )(x, id)


#define print_db(x) \
    _Generic((x), \
        datablock_t *:  print_block, \
        dataframe_t *:  print_frame, \
        btree_t *:      print_btree, \
        hashmap_t *:    print_map    \
    )(x)

#endif

