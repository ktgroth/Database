
#include <stdio.h>

#include "include/dataframe.h"
#include "include/datablock.h"
#include "include/btree.h"

#define UNUSED(x) (void)(x);


int main(int argc, char *argv[])
{
    UNUSED(argc)
    UNUSED(argv);

    const char *colnames[] = { "fname", "lname", "age", "salary" };
    const type_e coltypes[] = { COL_STRING, COL_STRING, COL_INT, COL_FLOAT };
    dataframe_t *frame = init_frame(4, colnames, coltypes);

    void *block1[] = { "Kasen", "Groth", (int []){ 22 }, (double []){ 65000 } };
    void *block2[] = { "Nuno", "Alves", (int []){ 22 }, (double []){ 60000 } };
    void *block3[] = { "Christian", "Groth", (int []){ 23 }, (double []){ 75000 } };
    void *block4[] = { "Joseph", "Speidel", (int []){ 22 }, (double []){ 100000 } };
    void *block5[] = { "Camryn", "Groth", (int []){ 26 }, (double []){ 68000 } };
    void *block6[] = { "Katie", "Smith", (int []){ 28 }, (double []){ 48000 } };
    void *block7[] = { "Noah", "Smith", (int []){ 27 }, (double []){ 88000 } };

    datablock_t *row1 = init_block(frame, block1);
    datablock_t *row2 = init_block(frame, block2);
    datablock_t *row3 = init_block(frame, block3);
    datablock_t *row4 = init_block(frame, block4);
    datablock_t *row5 = init_block(frame, block5);
    datablock_t *row6 = init_block(frame, block6);
    datablock_t *row7 = init_block(frame, block7);

    frame_add(frame, row1);
    frame_add(frame, row2);
    frame_add(frame, row3);
    puts("=== Frame Add ===");
    print_frame(frame);
    puts("\n");


    frame_update(frame, "fname", "Kasen", "salary", (double []){ 120000 });
    puts("=== Frame Update ===");
    print_frame(frame);
    puts("\n");


    frame_add(frame, row4);
    frame_add(frame, row5);
    frame_add(frame, row6);
    frame_add(frame, row7);
    puts("=== Frame Add ===");
    print_frame(frame);
    puts("\n");


    frame_update(frame, "fname", "Katie", "lname", "Groth");
    puts("=== Frame Update ===");
    print_frame(frame);
    puts("\n");


    dataframe_t *groths = frame_search(frame, "lname", "Groth");
    puts("=== Frame Search ===");
    print_frame(groths);
    puts("\n");

    btree_t *tree = init_btree(2, "id", COL_INT);
    btree_add(tree, (void *)(int []){ 1 }, row1);
    btree_add(tree, (void *)(int []){ 2 }, row2);
    btree_add(tree, (void *)(int []){ 3 }, row3);
    btree_add(tree, (void *)(int []){ 4 }, row4);
    btree_add(tree, (void *)(int []){ 5 }, row5);
    btree_add(tree, (void *)(int []){ 6 }, row6);
    btree_add(tree, (void *)(int []){ 7 }, row7);

    puts("=== Btree Add ===");
    print_btree(tree);
    puts("\n");


    frame_update(frame, "fname", "Kasen", "salary", (double []){ 125000 });
    puts("=== Frame Update ===");
    print_frame(frame);
    puts("\n\n=== Btree ===");
    print_btree(tree);

    return 0;
}

