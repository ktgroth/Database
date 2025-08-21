
#include <stdio.h>
#include <stdlib.h>

#include "include/map.h"

hashmap_t *init_map(size_t size)
{
    hashmap_t *map = (hashmap_t *)calloc(1, sizeof(hashmap_t));
    if (!map)
    {
        perror("map = calloc(1, sizeof(hashmap_t))");
        return NULL;
    }

    map->size = size;
    map->pairs = (pair_t **)calloc(size, sizeof(pair_t *));
    if (!map->pairs)
    {
        perror("pairs = calloc(size, sizeof(pair_t))");
        free(map);
        return NULL;
    }

    return map;
}

size_t hashs(char *id, size_t size)
{
    size_t h = 0;
    for (size_t i = 0; i < strlen(id); ++i)
        h += id[i];

    return h % size;
}

size_t hashi(size_t id, size_t size)
{
    return id % size;
}

void map_add(hashmap_t *map, db_id_t id, int value)
{
    size_t h = hash(id, map->size);

    if (!map->pairs[h])
        map->pairs[h] = (pair_t *)calloc(1, sizeof(pair_t));
    pair_t *p = map->pairs[h];

    while (p->next)
        p = p->next;

    pair_t *n = (pair_t *)calloc(1, sizeof(pair_t));
    n->id = id;
    n->value = value;

    p->next = n;
}

void map_remove(hashmap_t *map, db_id_t id)
{
    size_t h = hash(id, map->size);

    if (!map->pairs[h])
        return;

    pair_t *p = map->pairs[h];
    pair_t *t = p;
    while (p && cmp(p->id, id))
    {
        t = p;
        p = p->next;
    }

    if (!p)
        return;

    t->next = p->next;

    p->next = NULL;
    free(p);
}

int map_search(hashmap_t *map, db_id_t id)
{
    size_t h = hash(id, map->size);

    if (!map->pairs[h])
        return 0;

    pair_t *p = map->pairs[h];
    while (p && cmp(p->id, id))
        p = p->next;

    if (!p)
        return 0;

    return p->value;
}

