
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
    bt->keys = (int *)calloc(2*u - 1, sizeof(int));

    bt->children = (btree_t **)calloc(2*u, sizeof(btree_t *));

    bt->is_leaf = is_leaf;
    bt->u = u;

    return bt;
}

void print_btree(btree_t *root, size_t level)
{
    if (!root)
        return;

    for (size_t i = 0; i < root->nkeys; ++i)
    {
        if (!root->is_leaf)
            print_btree(root->children[i], level + 1);

        for (int j = 0; j < level; ++j)
            printf("\t");
        printf("%d\n", root->keys[i]);
    }

    if (!root->is_leaf)
        print_btree(root->children[root->nkeys], level + 1);
}

void btree_split_child(btree_t *parent, size_t i)
{
    btree_t *child = parent->children[i];
    size_t u = child->u;

    int mkey = child->keys[u - 1];
    for (size_t j = parent->nkeys; j > i + 1; --j)
        parent->keys[j] = parent->keys[j - 1];
    parent->keys[i] = mkey;
    ++parent->nkeys;

    btree_t *new_child = init_btree(child->is_leaf, u);
    for (size_t j = parent->nkeys; j > i + 1; --j)
        parent->children[j] = parent->children[j - 1];
    parent->children[i + 1] = new_child;

    child->nkeys = u - 1;
    new_child->nkeys = u - 1;
    for (size_t j = 0; j < u; ++j)
        new_child->keys[j] = child->keys[u + j];

    if (!child->is_leaf)
    {
        for (size_t j = 0; j <= u; ++j)
        {
            new_child->children[j] = child->children[u + j];
            child->children[u + j] = NULL;
        }
    }
}

void btree_add(btree_t **root, int key)
{
    if (!root || !*root)
        return;

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
        while (idx > 0 && curr->keys[idx - 1] > key)
        {
            curr->keys[idx] = curr->keys[idx - 1];
            --idx;
        }

        if (curr->children[idx]->nkeys == t)
        {
            btree_split_child(curr, idx);
            if (key > curr->keys[idx])
                ++idx;
        }

        curr = curr->children[idx];
    }

    size_t idx = curr->nkeys;
    while (idx > 0 && curr->keys[idx - 1] > key)
    {
        curr->keys[idx] = curr->keys[idx - 1];
        --idx;
    }

    curr->keys[idx] = key;
    ++curr->nkeys;
}


void btree_delete_merge(btree_t *node, size_t i, size_t j);
void btree_delete_sibling(btree_t *node, size_t i, size_t j);
void btree_delete_internal(btree_t *node, int key, size_t i);

int btree_delete_predecessor(btree_t *node)
{
    if (node->is_leaf)
        return node->keys[--node->nkeys];


}

int btree_delete_successor(btree_t *node)
{

}

void btree_delete_internal(btree_t *node, int key, size_t i)
{
    size_t u = node->u;

    if (node->is_leaf)
    {
        if (i < node->nkeys && node->keys[i] == key)
        {
            for (; i < node->nkeys - 1; ++i)
                node->keys[i] = node->keys[i + 1];
            --node->nkeys;
        }

        return;
    }

    if (node->children[i]->nkeys >= u)
    {
        node->keys[i] = btree_delete_predecessor(node->children[i]);
        return;
    } else if (node->children[i + 1]->nkeys >= u)
    {
        node->keys[i] = btree_delete_successor(node->children[i + 1]);
        return;
    }

    btree_delete_merge(node, i, i + 1);
    btree_delete_internal(node->children[i], key, u - 1);
}

void btree_delete_sibling(btree_t *node, size_t i, size_t j)
{

}

void btree_delete_merge(btree_t *node, size_t i, size_t j)
{

}

void btree_remove(btree_t **root, int key)
{
    if (!root || !*root)
        return;

    btree_t *curr = *root;
    size_t u = curr->u;
    size_t t = 2*u - 1;

    while (curr)
    {
        size_t idx = 0;
        while (idx < curr->nkeys && curr->keys[idx] < key)
            ++idx;

        if (curr->is_leaf)
        {
            if (idx < curr->nkeys && curr->keys[idx] == key)
            {
                for (; idx < curr->nkeys - 1; ++idx)
                    curr->keys[idx] = curr->keys[idx + 1];
                --curr->nkeys;
            }

            return;
        }

        if (idx < curr->nkeys && curr->keys[idx] == key)
        {
            btree_delete_internal(curr, key, idx);
            return;
        } else if (curr->children[idx]->nkeys >= u);
        else
        {
            if (idx != 0 && idx + 2 <= curr->nkeys && curr->children[idx + 2])
            {
                if (curr->children[idx - 1]->nkeys >= u)
                    btree_delete_sibling(curr, idx, idx - 1);
                else if (curr->children[idx + 1]->nkeys >= u)
                    btree_delete_sibling(curr, idx, idx + 1);
                else
                    btree_delete_merge(curr, idx, idx + 1);
            } else if (idx == 0)
            {
                if (curr->children[idx + 1]->nkeys >= u)
                    btree_delete_sibling(curr, idx, idx + 1);
                else
                    btree_delete_merge(curr, idx, idx + 1);
            } else if (idx + 1 <= curr->nkeys && curr->children[idx + 1])
            {
                if (curr->children[idx - 1]->nkeys >= u)
                    btree_delete_sibling(curr, idx, idx - 1);
                else
                    btree_delete_merge(curr, idx, idx - 1);
            }
        }

        curr = curr->children[idx];
    }
}

int btree_search(btree_t *root, int key)
{

}

