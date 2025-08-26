
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "include/btree.h"


btree_t *init_btree(int is_leaf, size_t u)
{
    btree_t *bt = (btree_t *)calloc(1, sizeof(btree_t));
    if (!bt)
    {
        perror("bt = calloc(1, sizeof(btree_t))");
        return NULL;
    }

    bt->nkeys = 0;
    bt->keys = (db_id_t *)calloc(2*u - 1, sizeof(db_id_t));
    bt->dptr = (datablock_t **)calloc(2*u - 1, sizeof(datablock_t *));

    bt->children = (btree_t **)calloc(2*u, sizeof(btree_t *));

    bt->is_leaf = is_leaf;
    bt->u = u;

    return bt;
}

void free_btree(btree_t *node)
{
    if (!node)
        return;

    if (node->is_leaf)
    {
        for (size_t i = 0; i <= node->nkeys; ++i)
            free_btree(node->children[i]);
        free(node->children);
    }

    free(node->keys);
    free(node);
}

void print_btree(btree_t *root, size_t level)
{
    if (!root)
        return;

    for (size_t i = 0; i < root->nkeys; ++i)
    {
        if (!root->is_leaf)
            print_btree(root->children[i], level + 1);

        for (size_t j = 0; j < level; ++j)
            printf("\t");

        char buf[256];
        print_id(root->keys[i], buf, sizeof(buf));
        printf("%s\n", buf);
    }

    if (!root->is_leaf)
        print_btree(root->children[root->nkeys], level + 1);
}

db_id_t btree_key(btree_t *root)
{
    
}


void btree_split_child(btree_t *parent, size_t i)
{
    btree_t *child = parent->children[i];
    size_t u = child->u;

    btree_t *new_child = init_btree(child->is_leaf, u);
    new_child->nkeys = u - 1;

    for (size_t j = 0; j < u - 1; ++j)
        new_child->keys[j] = child->keys[u + j];

    if (!child->is_leaf)
    {
        for (size_t j = 0; j < u; ++j)
        {
            new_child->children[j] = child->children[u + j];
            child->children[u + j] = NULL;
        }
    }

    db_id_t mkey = child->keys[u - 1];
    child->nkeys = u - 1;

    for (size_t j = parent->nkeys + 1; j > i + 1; --j)
        parent->children[j] = parent->children[j - 1];
    parent->children[i + 1] = new_child;

    for (size_t j = parent->nkeys; j > i; --j)
        parent->keys[j] = parent->keys[j - 1];
    parent->keys[i] = mkey;

    ++parent->nkeys;
}

void btree_add(btree_t **root, datablock_t *block)
{
    if (!root || !*root)
        return;

    db_id_t key = btree_key(*root);
    btree_t *curr = *root;
    size_t u = curr->u;
    size_t t = 2*u - 1;

    if (curr->nkeys == t)
    {
        btree_t *new_root = init_btree(0, u);
        *root = new_root;

        new_root->children[0] = curr;
        btree_split_child(new_root, 0);
        curr = new_root;
    }

    while (!curr->is_leaf)
    {
        size_t idx = curr->nkeys;
        while (idx > 0 && GT(curr->keys[idx - 1], key))
            --idx;

        if (curr->children[idx]->nkeys == t)
        {
            btree_split_child(curr, idx);
            if (GT(key, curr->keys[idx]))
                ++idx;
        }

        curr = curr->children[idx];
    }

    size_t idx = curr->nkeys;
    while (idx > 0 && GT(curr->keys[idx - 1], key))
    {
        curr->keys[idx] = curr->keys[idx - 1];
        --idx;
    }

    curr->keys[idx] = key;
    ++curr->nkeys;
}

void btree_delete_merge(btree_t *node, size_t i, size_t j);
void btree_delete_sibling(btree_t *node, size_t i, size_t j);
void btree_delete_internal(btree_t *node, db_id_t key, size_t i);

db_id_t btree_get_predecessor(btree_t *node, size_t idx)
{
    node = node->children[idx];
    while (!node->is_leaf)
        node = node->children[node->nkeys];
    return node->keys[node->nkeys - 1];
}

db_id_t btree_get_successor(btree_t *node, size_t idx)
{
    node = node->children[idx + 1];
    while (!node->is_leaf)
        node = node->children[0];
    return node->keys[0];
}

void btree_merge(btree_t *parent, size_t idx)
{
    btree_t *child = parent->children[idx];
    btree_t *sibling = parent->children[idx + 1];

    child->keys[child->nkeys] = parent->keys[idx];
    for (size_t i = 0; i < sibling->nkeys; ++i)
        child->keys[child->nkeys + 1 + i] = sibling->keys[i];

    if (!child->is_leaf)
    {
        for (size_t i = 0; i <= sibling->nkeys; ++i)
        {
            child->children[child->nkeys + 1 + i] = sibling->children[i];
            sibling->children[i] = NULL;
        }
    }

    child->nkeys += 1 + sibling->nkeys;
    for (size_t i = idx; i + 1 < parent->nkeys; ++i)
        parent->keys[i] = parent->keys[i + 1];
    for (size_t i = idx + 1; i + 1 <= parent->nkeys; ++i)
        parent->children[i] = parent->children[i + 1];

    parent->children[parent->nkeys] = NULL;
    --parent->nkeys;

    sibling->nkeys = 0;
    free_btree(sibling);
}

void btree_fill(btree_t *node, size_t idx)
{
    size_t u = node->u;

    if (idx != 0 && node->children[idx - 1] && node->children[idx - 1]->nkeys >= u)
    {
        btree_t *child = node->children[idx];
        btree_t *left = node->children[idx - 1];

        for (size_t i = child->nkeys; i > 0; --i)
            child->keys[i] = child->keys[i - 1];
        if (!child->is_leaf)
        {
            for (size_t i = child->nkeys + 1; i > 0; --i)
                child->children[i] = child->children[i - 1];
            child->children[0] = left->children[left->nkeys];
            left->children[left->nkeys] = NULL;
        }

        child->keys[0] = node->keys[idx - 1];
        node->keys[idx - 1] = left->keys[left->nkeys - 1];

        ++child->nkeys;
        --left->nkeys;
        return;
    }

    if (idx != node->nkeys && node->children[idx + 1] && node->children[idx + 1]->nkeys >= u)
    {
        btree_t *child = node->children[idx];
        btree_t *right = node->children[idx + 1];

        child->keys[child->nkeys] = node->keys[idx];
        if (!child->is_leaf)
        {
            child->children[child->nkeys + 1] = right->children[0];
            for (size_t i = 0; i + 1 <= right->nkeys; ++i)
                right->children[i] = right->children[i + 1];
            right->children[right->nkeys] = NULL;
        }

        node->keys[idx] = right->keys[0];
        for (size_t i = 0; i + 1 < right->nkeys; ++i)
            right->keys[i] = right->keys[i + 1];

        ++child->nkeys;
        --right->nkeys;
        return;
    }

    if (idx < node->nkeys)
        btree_merge(node, idx);
    else
        btree_merge(node, idx - 1);
}

void btree_remove(btree_t **root, db_id_t key)
{
    if (!root || !*root)
        return;

    btree_t *curr = *root;
    size_t u = curr->u;

    while (curr)
    {
        size_t idx = 0;
        while (idx < curr->nkeys && LT(curr->keys[idx], key))
            ++idx;

        if (idx < curr->nkeys && EQ(curr->keys[idx], key))
        {
            if (curr->is_leaf)
            {
                for (size_t i = idx; i + 1 < curr->nkeys; ++i)
                    curr->keys[i] = curr->keys[i + 1];
                --curr->nkeys;

                break;
            } else
            {
                if (curr->children[idx]->nkeys >= u)
                {
                    db_id_t pred = btree_get_predecessor(curr, idx);
                    curr->keys[idx] = pred;
                    key = pred;
                } else if (curr->children[idx + 1]->nkeys >= u)
                {
                    db_id_t succ = btree_get_successor(curr, idx);
                    curr->keys[idx++] = succ;
                    key = succ;
                } else
                    btree_merge(curr, idx);
            }
        } else
        {
            if (curr->children[idx]->nkeys < u)
                btree_fill(curr, idx);

            if (idx > curr->nkeys)
                idx = curr->nkeys;
        }

        curr = curr->children[idx];
    }

    if ((*root)->nkeys == 0)
    {
        btree_t *old = *root;
        if ((*root)->is_leaf);
        else
            *root = (*root)->children[0];
        free_btree(old);
    }
}

void btree_change(btree_t *root, db_id_t key, datablock_t *block)
{

}

datablock_t *btree_search(btree_t *root, db_id_t key)
{
    if (!root)
        return NULL;

    btree_t *curr = root;
    while (curr)
    {
        size_t idx = 0;
        while (idx < curr->nkeys && LT(curr->keys[idx], key))
            ++idx;

        if (idx < curr->nkeys && EQ(curr->keys[idx], key))
            return curr->dptr[idx];

        curr = curr->children[idx];
    }

    return NULL;
}

