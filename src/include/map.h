
#ifndef GDB_MAP_
#define GDB_MAP_

#include <string.h>

#include "id.h"
#include "data.h"

typedef struct pair
{
    db_id_t id;
    datablock_t *value;
    struct pair *next;
} pair_t;

typedef struct
{
    size_t size;
    pair_t **pairs;
} hashmap_t;


hashmap_t *init_map(size_t size);

size_t hashs(char *id, size_t size);
size_t hashi(size_t id, size_t size);

static inline size_t hash(db_id_t id, size_t size)
{
    switch (id.type)
    {
        case ID_INT:    return hashi(id.iid, size);
        case ID_CHAR:   return hashs(id.cid, size);
    }
    assert(0);
}


void map_add(hashmap_t *map, db_id_t id, datablock_t *value);
void map_remove(hashmap_t *map, db_id_t id);
datablock_t *map_search(hashmap_t *map, db_id_t id);

#endif

