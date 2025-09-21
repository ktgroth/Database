
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


bptree_t *init_bptree(size_t u, size_t ncols, char **colnames, type_e *coltypes, char *pkname, type_e pktype)
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

    print_bptree_node(tree->first_leaf, tree->pktype);
}

int bptree_insert(bptree_t *tree, void *key, void **values)
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

    child->nkeys = u - !child->is_leaf;
    for (size_t j = 0; j < new_child->nkeys; ++j)
    {
        new_child->keys[j] = child->keys[u + j];
        new_child->dptr[j] = child->dptr[u + j];
    }

    if (!child->is_leaf)
    {
        for (size_t j = 0; j <= new_child->nkeys; ++j)
        {
            new_child->children[j] = child->children[u + j];
            child->children[u + j] = NULL;
        }
    }

    void *mkey = child->keys[u - 1];
    datablock_t *block = child->dptr[u - 1];

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

static int check_cols(const bptree_t *tree, const datablock_t *block)
{
    for (size_t i = 0; i < MIN(tree->ncols, block->ncols); ++i)
        if (strcmp(tree->colnames[i], block->cols[i]->name))
            return 0;

    if (tree->ncols != block->ncols)
        return 0;

    return 1;
}

static column_t **update_column_metadata(const datablock_t *orig, const bptree_t *tree, const void *key)
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
        char *colname = tree->colnames[i];
        column_t *src_col = NULL;

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

    datablock_t *new_block = (datablock_t *)calloc(1, sizeof(datablock_t));
    if (!new_block)
    {
        perror("new_block = calloc(1, sizeof(datablock_t))");
        for (size_t i = 0; i < tree->ncols; ++i)
            free_column(new_cols[i]);
        free(new_cols);
        return NULL;
    }

    new_block->cols = new_cols;
    new_block->ncols = tree->ncols;

    return new_block;
}

int bptree_add(bptree_t *tree, void *key, datablock_t *block)
{
    if (!tree || !key || !block)
        return 0;

    if (!check_cols(tree, block))
    {
        datablock_t *new_block = update_block_structure(block, tree, key);
        if (!new_block)
            return 0;

        free_block(block);
        block = new_block;
    }

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

void bptree_delete_merge(bptree_node_t *node, size_t i, size_t j);
void bptree_delete_sibling(bptree_node_t *node, size_t i, size_t j);
void bptree_delete_internal(bptree_node_t *node, void * key, size_t i);

struct key_dptr_pair
{
    void        *key;
    datablock_t *dptr;
};

struct key_dptr_pair bptree_get_predecessor(bptree_node_t *node, size_t idx)
{
    node = node->children[idx];
    while (!node->is_leaf)
        node = node->children[node->nkeys - 1];
    return (struct key_dptr_pair){
        .key=node->keys[node->nkeys - 1],
        .dptr=node->dptr[node->nkeys - 1]
    };
}

struct key_dptr_pair bptree_get_successor(bptree_node_t *node, size_t idx)
{
    node = node->children[idx + 1];
    while (!node->is_leaf)
        node = node->children[0];
    return (struct key_dptr_pair){
        .key=node->keys[0],
        .dptr=node->dptr[0]
    };
}

void bptree_merge(bptree_node_t *parent, size_t idx)
{
    bptree_node_t *child = parent->children[idx];
    bptree_node_t *sibling = parent->children[idx + 1];

    child->keys[child->nkeys] = parent->keys[idx];
    child->dptr[child->nkeys] = parent->dptr[idx];
    for (size_t i = 0; i < sibling->nkeys; ++i)
    {
        child->keys[child->nkeys + 1 + i] = sibling->keys[i];
        child->dptr[child->nkeys + 1 + i] = sibling->dptr[i];
    }

    if (!child->is_leaf)
    {
        for (size_t i = 0; i <= sibling->nkeys; ++i)
        {
            child->children[child->nkeys + 1 + i] = sibling->children[i];
            sibling->children[i] = NULL;
        }
    }

    child->nkeys += 1 + sibling->nkeys;
    child->next = sibling->next;
    for (size_t i = idx; i + 1 < parent->nkeys; ++i)
    {
        parent->keys[i] = parent->keys[i + 1];
        parent->dptr[i] = parent->dptr[i + 1];
    }

    for (size_t i = idx + 1; i + 1 <= parent->nkeys; ++i)
        parent->children[i] = parent->children[i + 1];

    parent->children[parent->nkeys] = NULL;
    --parent->nkeys;

    sibling->nkeys = 0;
    free_bptree_node(sibling);
}

void bptree_fill(bptree_node_t *node, size_t idx)
{
    size_t u = node->u;

    if (idx != 0 && node->children[idx - 1] && node->children[idx - 1]->nkeys >= u)
    {
        bptree_node_t *child = node->children[idx];
        bptree_node_t *left = node->children[idx - 1];

        for (size_t i = child->nkeys; i > 0; --i)
        {
            child->keys[i] = child->keys[i - 1];
            child->dptr[i] = child->dptr[i - 1];
        }

        if (!child->is_leaf)
        {
            for (size_t i = child->nkeys + 1; i > 0; --i)
                child->children[i] = child->children[i - 1];
            child->children[0] = left->children[left->nkeys];
            left->children[left->nkeys] = NULL;
        }

        child->keys[0] = node->keys[idx - 1];
        child->dptr[0] = node->dptr[idx - 1];

        node->keys[idx - 1] = left->keys[left->nkeys - 1];
        node->dptr[idx - 1] = left->dptr[left->nkeys - 1];

        ++child->nkeys;
        --left->nkeys;
        return;
    }

    if (idx != node->nkeys && node->children[idx + 1] && node->children[idx + 1]->nkeys >= u)
    {
        bptree_node_t *child = node->children[idx];
        bptree_node_t *right = node->children[idx + 1];

        child->keys[child->nkeys] = node->keys[idx];
        child->dptr[child->nkeys] = node->dptr[idx];

        if (!child->is_leaf)
        {
            child->children[child->nkeys + 1] = right->children[0];
            for (size_t i = 0; i + 1 <= right->nkeys; ++i)
                right->children[i] = right->children[i + 1];
            right->children[right->nkeys] = NULL;
        }

        node->keys[idx] = right->keys[0];
        node->dptr[idx] = right->dptr[0];

        for (size_t i = 0; i + 1 < right->nkeys; ++i)
        {
            right->keys[i] = right->keys[i + 1];
            right->dptr[i] = right->dptr[i + 1];
        }

        ++child->nkeys;
        --right->nkeys;
        return;
    }

    if (idx < node->nkeys)
        bptree_merge(node, idx);
    else
        bptree_merge(node, idx - 1);
}

static size_t tabs = 0;
extern void test_print(bptree_node_t *node, type_e type)
{
    if (!node)
        return;

    for (size_t i = 0; i < node->nkeys; ++i)
    {
        ++tabs;
        if (!node->is_leaf)
            test_print(node->children[i], type);
        --tabs;

        for (size_t j = 0; j < tabs; ++j)
            printf("\t");
        print_value(type, node->keys[i]);
        puts("");
    }

    ++tabs;
    if (!node->is_leaf)
        test_print(node->children[node->nkeys], type);
    --tabs;
}

int bptree_remove_from_leaf(bptree_t *tree, void *key)
{
    if (!tree || !key)
        return 0;

    bptree_node_t *parent = tree->root;
    bptree_node_t *curr = tree->root;

    size_t idx;
    while (!curr->is_leaf)
    {
        parent = curr;

        idx = 0;
        while (idx < curr->nkeys && LT(tree->pktype, curr->keys[idx], key))
            ++idx;

        curr = curr->children[idx];
    }

    

    return 1;
}



int bptree_remove_key(bptree_t *tree, void *key)
{
    if (!tree)
        return 0;

    test_print(tree->root, tree->pktype);
    puts("\n");
    bptree_node_t *curr = tree->root;
    size_t u = curr->u;

    if (!bptree_remove_from_leaf(tree, key))
        return 0;

    while (curr)
    {
        size_t idx = 0;
        while (idx < curr->nkeys && LT(tree->pktype, curr->keys[idx], key))
            ++idx;

        if (idx < curr->nkeys && EQ(tree->pktype, curr->keys[idx], key))
        {
            if (curr->is_leaf)
            {
                for (size_t i = idx; i + 1 < curr->nkeys; ++i)
                {
                    curr->keys[i] = curr->keys[i + 1];
                    curr->dptr[i] = curr->dptr[i + 1];
                }

                --curr->nkeys;

                break;
            } else
            {
                if (curr->children[idx]->nkeys >= u)
                {
                    struct key_dptr_pair pred = bptree_get_predecessor(curr, idx);
                    curr->keys[idx] = pred.key;
                    curr->dptr[idx] = pred.dptr;

                    key = pred.key;
                } else if (curr->children[idx + 1]->nkeys >= u)
                {
                    struct key_dptr_pair succ = bptree_get_successor(curr, idx);
                    curr->keys[idx] = succ.key;
                    curr->dptr[idx++] = succ.dptr;

                    key = succ.key;
                } else
                    bptree_merge(curr, idx);
            }
        } else
        {
            if (curr->is_leaf)
                break;

            if (curr->children[idx]->nkeys < u)
                bptree_fill(curr, idx);

            if (idx > curr->nkeys)
                idx = curr->nkeys;
        }

        curr = curr->children[idx];
    }

    if (tree->root->nkeys == 0)
    {
        bptree_node_t *old = tree->root;
        if (!old->is_leaf)
            tree->root = old->children[0];

        old->is_leaf = 1;
        old->children = NULL;
        free_bptree_node(old);
    }

    test_print(tree->root, tree->pktype);
    puts("\n");

    return 1;
}

struct key_list
{
    size_t    nkeys;
    void    **keys;
};

struct key_list bptree_find(bptree_node_t *node, size_t colidx, const void *value)
{
    if (!node || !value)
        return (struct key_list){ .nkeys=0, .keys=NULL };

    struct key_list keys = { .nkeys=0, .keys=NULL };
    if (node->is_leaf)
    {
        for (size_t i = 0; i < node->nkeys; ++i)
        {
            if (!column_cmp_value(node->dptr[i]->cols[colidx], value))
            {
                void **new_keys = realloc(keys.keys, (keys.nkeys + 1) * sizeof(void *));
                if (!new_keys)
                    continue;

                keys.keys = new_keys;
                keys.keys[keys.nkeys++] = node->keys[i];
            }
        }
    } else
    {
        for (size_t i = 0; i <= node->nkeys; ++i)
        {
            struct key_list ckeys = bptree_find(node->children[i], colidx, value);
            if (ckeys.nkeys)
            {
                void **new_keys = realloc(keys.keys, (keys.nkeys + ckeys.nkeys) * sizeof(void *));
                if (new_keys)
                {
                    keys.keys = new_keys;
                    for (size_t j = 0; j < ckeys.nkeys; ++j)
                        keys.keys[keys.nkeys + j] = ckeys.keys[j];
                    keys.nkeys += ckeys.nkeys;
                }

                free(ckeys.keys);
            }
        }
    }

    return keys;
}

datablock_t *bptree_get(const bptree_t *tree, const void *key)
{
    if (!tree || !key)
    {
        perror("!tree || !key");
        return NULL;
    }

    bptree_node_t *curr = tree->root;
    while (curr)
    {
        size_t idx = 0;
        while (idx < curr->nkeys && GT(tree->pktype, key, curr->keys[idx]))
            ++idx;

        if (idx < curr->nkeys && EQ(tree->pktype, key, curr->keys[idx]))
            return curr->dptr[idx];

        curr = curr->children[idx];
    }

    print_value(tree->pktype, key);
    fprintf(stderr, "not in btree.\n");
    return NULL;
}

int bptree_remove(bptree_t *btree, const char *colname, void *value)
{
    if (!btree || !colname || !value)
    {
        perror("!btree || !colname || !value");
        return 0;
    }

    if (!strcmp(colname, btree->pkname))
        return bptree_remove_key(btree, value);

    size_t idx = -1ULL;
    for (size_t i = 0; i < btree->ncols; ++i)
        if (!strcmp(colname, btree->colnames[i]))
        {
            idx = i;
            break;
        }

    if (idx == -1ULL)
    {
        fprintf(stderr, "%s not in btree.\n", colname);
        return 0;
    }

    struct key_list rkeys = bptree_find(btree->root, idx, value);
    for (size_t i = 0; i < rkeys.nkeys; ++i)
    {
        print_value(btree->pktype, rkeys.keys[i]);
        puts("");
        if (!bptree_remove_key(btree, rkeys.keys[i]))
            return 0;
    }

    return 1;
}

const dataframe_t *bptree_lookup_key(const bptree_t *tree, void *key)
{
    if (!tree || !key)
        return NULL;

    bptree_node_t *curr = tree->root;
    while (curr)
    {
        size_t idx = 0;
        while (idx < curr->nkeys && LT(tree->pktype, curr->keys[idx], key))
            ++idx;

        if (idx < curr->nkeys && EQ(tree->pktype, curr->keys[idx], key))
        {
            dataframe_t *lookup = init_frame(tree->ncols, tree->colnames, tree->coltypes);
            frame_add(lookup, curr->dptr[idx]);
            return lookup;
        }

        curr = curr->children[idx];
    }

    return NULL;
}

const dataframe_t *bptree_lookup(const bptree_t *tree, const char *colname, void *value)
{
    if (!tree || !colname || !value)
    {
        perror("!tree || !colname || !value");
        return NULL;
    }

    if (!strcmp(colname, tree->pkname))
        return bptree_lookup_key(tree, value);

    size_t idx = -1ULL;
    for (size_t i = 0; i < tree->ncols; ++i)
        if (!strcmp(colname, tree->colnames[i]))
        {
            idx = i;
            break;
        }

    if (idx == -1ULL)
        return NULL;

    struct key_list lkeys = bptree_find(tree->root, idx, value);
    dataframe_t *frame = init_frame(tree->ncols, tree->colnames, tree->coltypes);
    for (size_t i = 0; i < lkeys.nkeys; ++i)
        if (!frame_add(frame, bptree_get(tree, lkeys.keys[i])))
        {
            free_frame(frame);
            return NULL;
        }

    return frame;
}

int bptree_update_key(bptree_t *tree, const void *key, size_t colidx, void *value)
{
    if (!tree)
        return 0;

    bptree_node_t *curr = tree->root;
    while (curr)
    {
        size_t idx = 0;
        while (idx < curr->nkeys && LT(tree->pktype, curr->keys[idx], key))
            ++idx;

        if (idx < curr->nkeys && EQ(tree->pktype, curr->keys[idx], key))
            return column_update(curr->dptr[idx]->cols[colidx], value);

        curr = curr->children[idx];
    }

    return 0;
}

int bptree_update(bptree_t *tree, const char *keycol, const void *keyval, const char *colname, void *value)
{
    if (!tree || !colname || !value)
    {
        perror("!tree || !colname || !value");
        return 0;
    }

    size_t kidx = -1ULL, vidx = -1ULL;
    for (size_t i = 0; i < tree->ncols; ++i)
        if (!strcmp(keycol, tree->colnames[i]))
        {
            kidx = i;
            break;
        }

    for (size_t i = 0; i < tree->ncols; ++i)
        if (!strcmp(colname, tree->colnames[i]))
        {
            vidx = i;
            break;
        }

    if (kidx == -1ULL || vidx == -1ULL)
        return 0;

    struct key_list ukeys = bptree_find(tree->root, kidx, keyval);
    for (size_t i = 0; i < ukeys.nkeys; ++i)
        if (!bptree_update_key(tree, ukeys.keys[i], vidx, value))
            return 0;

    return 1;
}

