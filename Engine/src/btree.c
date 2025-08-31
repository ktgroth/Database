
#include <stdio.h>
#include <stdlib.h>

#include "include/btree.h"


btree_node_t *init_btree_node(int is_leaf, size_t u)
{
    btree_node_t *node = (btree_node_t *)calloc(1, sizeof(btree_node_t));
    if (!node)
    {
        perror("node = calloc(1, sizeof(btree_node_t))");
        return NULL;
    }

    node->nkeys = 0;
    node->keys = (void **)calloc(2*u - 1, sizeof(void *));
    node->dptr = (datablock_t **)calloc(2*u - 1, sizeof(datablock_t *));

    node->children = (btree_node_t **)calloc(2*u, sizeof(btree_node_t *));

    node->is_leaf = is_leaf;
    node->u = u;

    return node;
}

void free_btree_node(btree_node_t *node)
{
    if (!node)
        return;

    if (node->is_leaf)
    {
        for (size_t i = 0; i <= node->nkeys; ++i)
            free_btree_node(node->children[i]);
        free(node->children);
    }

    free(node->keys);
    free(node);
}

void print_btree_node(const btree_node_t *node, type_e type, size_t level)
{
    if (!node)
        return;

    for (size_t i = 0; i < node->nkeys; ++i)
    {
        if (!node->is_leaf)
            print_btree_node(node->children[i], type, level + 1);

        for (size_t j = 0; j < level; ++j)
            printf("\t");

        print_value(type, node->keys[i]);
        puts("");
        print_block(node->dptr[i]);
    }

    if (!node->is_leaf)
        print_btree_node(node->children[node->nkeys], type, level + 1);
}


btree_t *init_btree(size_t u, const char *colname, const type_e coltype)
{
    btree_t *tree = (btree_t *)calloc(1, sizeof(btree_t));
    if (!tree)
    {
        perror("tree = calloc(1, sizeof(btree_t))");
        return NULL;
    }

    tree->u = u;
    tree->colname = colname;
    tree->coltype = coltype;
    tree->root = init_btree_node(1, u);
    if (!tree->root)
    {
        free(tree);
        return NULL;
    }

    return tree;
}

void free_btree(btree_t *tree)
{
    if (!tree)
        return;

    free(tree->root);
    free(tree);
}

void print_btree(const btree_t *tree)
{
    if (!tree)
        return;

    print_btree_node(tree->root, tree->coltype, 0);
}


void btree_split_child(btree_node_t *parent, size_t i)
{
    btree_node_t *child = parent->children[i];
    size_t u = child->u;

    btree_node_t *new_child = init_btree_node(child->is_leaf, u);
    new_child->nkeys = u - 1;

    for (size_t j = 0; j < u - 1; ++j)
    {
        new_child->keys[j] = child->keys[u + j];
        new_child->dptr[j] = child->dptr[u + j];
    }

    if (!child->is_leaf)
    {
        for (size_t j = 0; j < u; ++j)
        {
            new_child->children[j] = child->children[u + j];
            child->children[u + j] = NULL;
        }
    }

    void *mkey = child->keys[u - 1];
    datablock_t *block = child->dptr[u - 1];
    child->nkeys = u - 1;

    for (size_t j = parent->nkeys + 1; j > i + 1; --j)
        parent->children[j] = parent->children[j - 1];
    parent->children[i + 1] = new_child;

    for (size_t j = parent->nkeys; j > i; --j)
    {
        parent->keys[j] = parent->keys[j - 1];
        parent->dptr[j] = parent->dptr[j - 1];
    }
    parent->keys[i] = mkey;
    parent->dptr[i] = block;

    ++parent->nkeys;
}

int btree_add(btree_t *tree, void *key, datablock_t *block)
{
    if (!tree)
        return 0;

    btree_node_t *curr = tree->root;
    size_t u = tree->u;
    size_t t = 2*u - 1;

    if (curr->nkeys == t)
    {
        btree_node_t *new_root = init_btree_node(0, u);
        tree->root = new_root;

        new_root->children[0] = curr;
        btree_split_child(new_root, 0);
        curr = new_root;
    }

    while (!curr->is_leaf)
    {
        size_t idx = curr->nkeys;
        while (idx > 0 && GT(tree->coltype, curr->keys[idx - 1], key))
            --idx;

        if (curr->children[idx]->nkeys == t)
        {
            btree_split_child(curr, idx);
            if (key > curr->keys[idx])
                ++idx;
        }

        curr = curr->children[idx];
    }

    size_t idx = curr->nkeys;
    while (idx > 0 && LT(tree->coltype, curr->keys[idx - 1], key))
    {
        curr->keys[idx] = curr->keys[idx - 1];
        curr->dptr[idx] = curr->dptr[idx - 1];
        --idx;
    }

    curr->keys[idx] = key;
    curr->dptr[idx] = block;
    ++curr->nkeys;

    return 1;
}

void btree_delete_merge(btree_node_t *node, size_t i, size_t j);
void btree_delete_sibling(btree_node_t *node, size_t i, size_t j);
void btree_delete_internal(btree_node_t *node, db_id_t key, size_t i);

void *btree_get_predecessor(btree_node_t *node, size_t idx)
{
    node = node->children[idx];
    while (!node->is_leaf)
        node = node->children[node->nkeys];
    return node->keys[node->nkeys - 1];
}

void *btree_get_successor(btree_node_t *node, size_t idx)
{
    node = node->children[idx + 1];
    while (!node->is_leaf)
        node = node->children[0];
    return node->keys[0];
}

void btree_merge(btree_node_t *parent, size_t idx)
{
    btree_node_t *child = parent->children[idx];
    btree_node_t *sibling = parent->children[idx + 1];

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
    free_btree_node(sibling);
}

void btree_fill(btree_node_t *node, size_t idx)
{
    size_t u = node->u;

    if (idx != 0 && node->children[idx - 1] && node->children[idx - 1]->nkeys >= u)
    {
        btree_node_t *child = node->children[idx];
        btree_node_t *left = node->children[idx - 1];

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
        btree_node_t *child = node->children[idx];
        btree_node_t *right = node->children[idx + 1];

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

int btree_remove(btree_t *tree, const void *key)
{
    if (!tree)
        return 0;

    btree_node_t *curr = tree->root;
    size_t u = curr->u;

    while (curr)
    {
        size_t idx = 0;
        while (idx < curr->nkeys && LT(tree->coltype, curr->keys[idx], key))
            ++idx;

        if (idx < curr->nkeys && EQ(tree->coltype, curr->keys[idx], key))
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
                    void *pred = btree_get_predecessor(curr, idx);
                    curr->keys[idx] = pred;
                    key = pred;
                } else if (curr->children[idx + 1]->nkeys >= u)
                {
                    void *succ = btree_get_successor(curr, idx);
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

    if (tree->root->nkeys == 0)
    {
        btree_node_t *old = tree->root;
        if (old->is_leaf);
        else
            tree->root = old->children[0];

        free(old->children);
        old->is_leaf = 1;
        old->children = NULL;
        free_btree_node(old);
    }

    return 1;
}

int btree_update(btree_t *tree, const void *key, const datablock_t *block)
{
    if (!tree)
        return 0;

    return 1;
}

const datablock_t *btree_search(const btree_t *tree, const void *key)
{
    if (!tree)
        return NULL;

    btree_node_t *curr = tree->root;
    while (curr)
    {
        size_t idx = 0;
        while (idx < curr->nkeys && LT(tree->coltype, curr->keys[idx], key))
            ++idx;

        if (idx < curr->nkeys && EQ(tree->coltype, curr->keys[idx], key))
            return curr->dptr[idx];

        curr = curr->children[idx];
    }

    return NULL;
}

