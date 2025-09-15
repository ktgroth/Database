
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/storage.h"


storage_t *init_storage(storage_type_e type, size_t ncols, const char **colnames, const type_e *coltypes, const char *pkname, const type_e pktype)
{
    storage_t *store = (storage_t *)calloc(1, sizeof(storage_t));
    if (!store)
    {
        perror("store = calloc(1, sizeof(storage_t))");
        return NULL;
    }

    store->type = type;
    switch (type)
    {
        case STORAGE_FRAME:
            store->frame = init_frame(ncols, colnames, coltypes);
            break;

        case STORAGE_BTREE:
            store->btree = init_btree(80, ncols, colnames, coltypes, pkname, pktype);
            break;
    }

    return store;
}

void free_storage(storage_t *store)
{
    switch (store->type)
    {
        case STORAGE_FRAME:
            free_frame(store->frame);
            break;

        case STORAGE_BTREE:
            free_btree(store->btree);
            break;
    }

    free(store);
}

int storage_insert(storage_t *store, const void *key, void **values)
{
    switch (store->type)
    {
        case STORAGE_FRAME:
            return frame_insert(store->frame, values);

        case STORAGE_BTREE:
            return btree_insert(store->btree, key, values);
    }

    return 0;
}

int storage_add(storage_t *store, const void *key, datablock_t *block)
{
    switch (store->type)
    {
        case STORAGE_FRAME:
            return frame_add(store->frame, block);

        case STORAGE_BTREE:
            return btree_add(store->btree, key, block);
    }

    return 0;
}

int storage_remove(storage_t *store, const char *colname, void *value)
{
    switch (store->type)
    {
        case STORAGE_FRAME:
            return frame_remove(store->frame, colname, value);

        case STORAGE_BTREE:
            return btree_remove(store->btree, colname, value);
    }

    return 0;
}

const dataframe_t *storage_lookup(const storage_t *store, const char *colname, void *value)
{
    switch (store->type)
    {
        case STORAGE_FRAME:
            return frame_lookup(store->frame, colname, value);

        case STORAGE_BTREE:
            return btree_lookup(store->btree, colname, value);
    }

    return NULL;
}

int storage_update(storage_t *store, const char *keycol, const void *keyval, const char *colname, void *value)
{
    switch (store->type)
    {
        case STORAGE_FRAME:
            return frame_update(store->frame, keycol, keyval, colname, value);

        case STORAGE_BTREE:
            return btree_update(store->btree, keycol, keyval, colname, value);
    }

    return 0;
}

