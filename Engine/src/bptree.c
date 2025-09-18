
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/bptree.h"

static datablock_t *construct_block(const bptree_t *tree, void **values)
{
    return init_block(tree->ncols, tree->colnames, tree->coltypes, values);
}

bptree_node_t *init_bptree_node(int is_leaf, size_t u)
{
    bptree_node_t *node = (bptree_node_t *)calloc(1, sizeof(bptree_node_t));
    if (!node)
    {
        perror("node = calloc(1, sizeof(bptree_node_t))");
        return NULL;
    }

    node->nkeys = 0;
    node->keys = (void **)calloc(2*u - 1, sizeof(void *));
    if (!node->keys)
    {
        perror("node->keys = calloc(2*u - 1, sizeof(void *))");
        free(node);
        return NULL;
    }

    node->dptr = (datablock_t **)calloc(2*u - 1, sizeof(datablock_t *));
    if (!node->dptr)
    {
        perror("node->dptr = calloc(2*u - 1, sizeof(datablock_t *))");
        free(node->keys);
        free(node);
        return NULL;
    }

    node->children = (bptree_node_t **)calloc(2*u, sizeof(bptree_node_t *));
    if (!node->children)
    {
        perror("node->children = calloc(2*u, sizeof(bptree_node_t *))");
        free(node->keys);
        free(node->dptr);
        free(node);
        return NULL;
    }

    node->next = NULL;

    node->is_leaf = is_leaf;
    node->u = u;

    return node;
}

void free_bptree_node(bptree_node_t *node)
{
    if (!node)
        return;

    if (node->is_leaf)
    {
        for (size_t i = 0; i < node->nkeys; ++i)
        {
            free(node->keys[i]);
            free_block(node->dptr[i]);
        }
    } else
    {
        for (size_t i = 0; i <= node->nkeys; ++i)
            free_bptree_node(node->children[i]);
    }

    free(node->keys);
    free(node->dptr);
    free(node->children);

    free(node);
}

void print_bptree_node(const bptree_node_t *node, type_e type)
{
    if (!node)
        return;

    if (!node->is_leaf)
    {
        print_bptree_node(node->children[0], type);
        return;
    }

    for (size_t i = 0; i < node->nkeys; ++i)
    {
        print_value(type, node->keys[i]);
        datablock_t *data = node->dptr[i];
        for (size_t j = 0; j < data->ncols; ++j)
            print_value(data->cols[j]->field->type, data->cols[j]->field->value);
        puts("");
    }

    print_bptree_node(node->next, type);
}


bptree_t *init_bptree(size_t u, size_t ncols, const char **colnames, const type_e *coltypes, const char *pkname, const type_e pktype)
{
    bptree_t *tree = (bptree_t *)calloc(1, sizeof(bptree_t));
    if (!tree)
    {
        perror("tree = calloc(1, sizeof(bptree_t))");
        return NULL;
    }

    tree->u = u;
    tree->ncols = ncols;
    tree->colnames = colnames;
    tree->coltypes = coltypes;
    tree->pkname = pkname;
    tree->pktype = pktype;

    tree->root = init_bptree_node(1, u);
    tree->first_leaf = tree->root;
    if (!tree->root)
    {
        free(tree);
        return NULL;
    }

    return tree;
}

void free_bptree(bptree_t *tree)
{
    if (!tree)
        return;

    free_bptree_node(tree->root);
    free(tree);
}

void print_bptree(const bptree_t *tree)
{
    if (!tree)
        return;

    datablock_t *block = tree->root->dptr[0];
    if (!block)
        return;

    printf(" %-15s ", tree->pkname);
    for (size_t i = 0; i < block->ncols; ++i)
        printf(" %-15s ", block->cols[i]->name);
    puts("");

    print_bptree_node(tree->root, tree->pktype);
}

int bptree_insert(bptree_t *tree, const void *key, void **values)
{
    if (!tree || !key || !values)
        return 0;

    datablock_t *block = construct_block(tree, values);
    return bptree_add(tree, key, block);
}

void bptree_split_child(bptree_node_t *parent, size_t i)
{
    bptree_node_t *child = parent->children[i];
    size_t u = child->u;

    bptree_node_t *new_child = init_bptree_node(child->is_leaf, u);
    new_child->nkeys = u - 1;
    new_child->next = child->next;
    child->next = new_child;

    for (size_t j = 0; j < new_child->nkeys; ++j)
    {
        new_child->keys[j] = child->keys[u + j];
        new_child->dptr[j] = child->dptr[u + j];
    }

    child->nkeys = u - !child->is_leaf;
    if (!child->is_leaf)
    {
        for (size_t j = 0; j < (2*u - 1) - child->nkeys; ++j)
        {
            new_child->children[j] = child->children[u + j];
            child->children[u + j] = NULL;
        }
    }

    void *mkey = child->keys[child->nkeys];
    datablock_t *block = child->dptr[child->nkeys];

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

#define MIN(x, y) ((x) > (y) ? y : x) 

static int check_cols(bptree_t *tree, datablock_t *block)
{
    for (size_t i = 0; i < MIN(tree->ncols, block->ncols); ++i)
        if (strcmp(tree->colnames[i], block->cols[i]->name))
            return 0;

    if (tree->ncols != block->ncols)
        return 0;

    return 1;
}

static column_t **update_column_metadata(const datablock_t *orig, const bptree_t *tree, void *key)
{
    size_t ncols = tree->ncols;
    column_t **new_cols = calloc(ncols, sizeof(column_t *));
    if (!new_cols)
        return NULL;

    datafield_t *pk_field = init_field(tree->pktype, key);
    if (!pk_field)
    {
        free(new_cols);
        return NULL;
    }

    new_cols[0] = init_column(tree->pkname, pk_field);
    if (!new_cols[0])
    {
        free_field(pk_field);
        free(new_cols);
        return NULL;
    }

    for (size_t i = 1; i < ncols; ++i)
    {
        const char *colname = tree->colnames[i];
        const column_t *src_col = NULL;

        for (size_t j = 0; j < orig->ncols; ++j)
        {
            if (!strcmp(orig->cols[j]->name, colname))
            {
                src_col = orig->cols[j];
                break;
            }
        }

        datafield_t *new_field = NULL;
        if (src_col)
            new_field = init_field(src_col->field->type, copy_key_value(src_col->field->type, src_col->field->value));
        else
            new_field = init_field(tree->coltypes[i], NULL);

        if (!new_field)
        {
            for (size_t k = 0; k < i; ++k)
                free_column(new_cols[k]);
            free(new_cols);
            return NULL;
        }

        new_cols[i] = init_column(colname, new_field);
        if (!new_cols[i])
        {
            free_field(new_field);
            for (size_t k = 0; k < i; ++k)
                free_column(new_cols[k]);
            free(new_cols);
            return NULL;
        }
    }

    return new_cols;
}

static datablock_t *update_block_structure(const datablock_t *orig, const bptree_t *tree, const void *key)
{
    if (!orig || !tree || !key)
        return NULL;

    column_t **new_cols = update_column_metadata(orig, tree, key);
    if (!new_cols)
        return NULL;

    datablock_t *new_block = init_block(tree->ncols, tree->colnames, tree->coltypes, NULL);
    if (!new_block)
    {
        for (size_t i = 0; i < tree->ncols; ++i)
            free_column(new_cols[i]);
        free(new_cols);
        return NULL;
    }

    new_block->cols = new_cols;
    new_block->ncols = tree->ncols;

    return new_block;
}

int bptree_add(bptree_t *tree, const void *key, datablock_t *block)
{
    if (!tree || !key || !block)
        return 0;

    if (!check_cols(tree, block) && !(block = update_block_structure(block, tree, key)))
        return 0;

    bptree_node_t *curr = tree->root;
    size_t u = tree->u;
    size_t t = 2*u - 1;

    if (curr->nkeys == t)
    {
        bptree_node_t *new_root = init_bptree_node(0, u);
        new_root->children[0] = curr;

        tree->root = new_root;
        bptree_split_child(new_root, 0);
        curr = new_root;
    }

    size_t idx;
    while (!curr->is_leaf)
    {
        idx = curr->nkeys;
        while (idx > 0 && GT(tree->pktype, curr->keys[idx - 1], key))
            --idx;

        if (curr->children[idx]->nkeys == t)
        {
            bptree_split_child(curr, idx);
            if (GT(tree->pktype, key, curr->keys[idx]))
                ++idx;
        }

        curr = curr->children[idx];
    }

    idx = curr->nkeys;
    while (idx > 0 && GT(tree->pktype, curr->keys[idx - 1], key))
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



