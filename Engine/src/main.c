
#include <stdio.h>

#include "include/database.h"
#include "include/bptree.h"


#define UNUSED(x) (void)(x);


int main(int argc, char *argv[])
{
    UNUSED(argc)
    UNUSED(argv);

    database_t *db = init_database("test");

    char *colnames[] = { "fname", "lname", "age", "salary" };
    type_e coltypes[] = { COL_STRING, COL_STRING, COL_INT8, COL_FLOAT64 };
    table_t *tbl = init_table("people", 4, colnames, coltypes, 1, 
                              "id", KEY_PK | COL_INT64);

    void *block1[] = { "Kasen", "Groth", (char []){ 22 }, (double []){ 65000 } };
    void *block2[] = { "Nuno", "Alves", (char []){ 22 }, (double []){ 60000 } };
    void *block3[] = { "Christian", "Groth", (char []){ 23 }, (double []){ 75000 } };
    void *block4[] = { "Joseph", "Speidel", (char []){ 22 }, (double []){ 100000 } };
    void *block5[] = { "Camryn", "Groth", (char []){ 26 }, (double []){ 68000 } };
    void *block6[] = { "Katie", "Smith", (char []){ 28 }, (double []){ 48000 } };
    void *block7[] = { "Noah", "Smith", (char []){ 27 }, (double []){ 88000 } };
    void *block8[] = { "Kyle", "Hammermueller", (char []){ 23 }, (double []){ 100000 } };
    void *block9[] = { "Liam", "Boyle", (char []){ 22 }, (double []){ 98000 } };

    char *colnames2[] = { "fname", "lname", "nkids", "salary", "age" };
    type_e coltypes2[] = { COL_STRING, COL_STRING, COL_INT8, COL_FLOAT64, COL_INT8 };
    void *block10[] = { "Heidi", "Groth", (char []){ 4 }, (double []){ 50000 }, (char []){ 62 } };
    void *block11[] = { "Richard", "Groth", (char []){ 4 }, (double []){ 250000 }, (char []){ 57 } };

    datablock_t *row1 = init_block(4, colnames, coltypes, block1);
    datablock_t *row2 = init_block(4, colnames, coltypes, block2);
    datablock_t *row3 = init_block(4, colnames, coltypes, block3);
    datablock_t *row4 = init_block(4, colnames, coltypes, block4);
    datablock_t *row5 = init_block(4, colnames, coltypes, block5);
    datablock_t *row6 = init_block(4, colnames, coltypes, block6);
    datablock_t *row7 = init_block(4, colnames, coltypes, block7);
    datablock_t *row8 = init_block(4, colnames, coltypes, block8);
    datablock_t *row9 = init_block(4, colnames, coltypes, block9);
    datablock_t *row10 = init_block(5, colnames2, coltypes2, block10);
    datablock_t *row11 = init_block(5, colnames2, coltypes2, block11);


    char *colnames3[] = { "id", "fname", "lname", "age", "salary" };
    type_e coltypes3[] = { COL_INT64, COL_STRING, COL_STRING, COL_INT8, COL_FLOAT64 };
    bptree_t *tree = init_bptree(2, 5, colnames3, coltypes3, "id", KEY_PK | COL_INT64);

    bptree_add(tree, (size_t []){ 1 }, row1);
    bptree_add(tree, (size_t []){ 2 }, row2);
    bptree_add(tree, (size_t []){ 3 }, row3);
    bptree_add(tree, (size_t []){ 4 }, row4);
    bptree_add(tree, (size_t []){ 5 }, row5);
    bptree_add(tree, (size_t []){ 6 }, row6);
    bptree_add(tree, (size_t []){ 7 }, row7);
    bptree_add(tree, (size_t []){ 8 }, row8);
    bptree_add(tree, (size_t []){ 9 }, row9);
    bptree_add(tree, (size_t []){ 10 }, row10);
    bptree_add(tree, (size_t []){ 11 }, row11);
    puts("=== B+Tree Add ===");
    print_bptree(tree);
    puts("\n");


    bptree_update(tree, "fname", "Katie", "lname", "Groth");
    puts("=== B+Tree Update ===");
    print_bptree(tree);
    puts("\n");


    dataframe_t *groths = bptree_lookup(tree, "lname", "Groth");
    puts("=== B+Tree Search ===");
    print_frame(groths);
    puts("\n");


    bptree_remove(tree, "id", (size_t []){ 4 });
    bptree_remove(tree, "lname", "Groth");
    puts("=== B+Tree Remove ===");
    print_bptree(tree);
    puts("\n");


    puts("=== Database Add ===");
    database_add_table(db, tbl);
    print_database(db);
    puts("\n");

    row1 = init_block(4, colnames, coltypes, block1);
    row2 = init_block(4, colnames, coltypes, block2);
    row3 = init_block(4, colnames, coltypes, block3);
    row4 = init_block(4, colnames, coltypes, block4);
    row5 = init_block(4, colnames, coltypes, block5);
    row6 = init_block(4, colnames, coltypes, block6);
    row7 = init_block(4, colnames, coltypes, block7);
    row8 = init_block(4, colnames, coltypes, block8);
    row9 = init_block(4, colnames, coltypes, block9);
    row10 = init_block(5, colnames2, coltypes2, block10);
    row11 = init_block(5, colnames2, coltypes2, block11);

    table_add(tbl, row1);
    table_add(tbl, row2);
    table_add(tbl, row3);
    table_add(tbl, row4);
    table_add(tbl, row5);
    table_add(tbl, row6);
    table_add(tbl, row7);
    puts("=== Table Add ===");
    print_table(tbl);
    puts("\n");


    table_update(tbl, "fname", "Katie", "lname", "Groth");
    puts("=== Table Update ===");
    print_table(tbl);
    puts("\n");


    groths = table_lookup(tbl, "lname", "Groth");
    puts("=== Table Search ===");
    print_frame(groths);
    puts("\n");


    table_add(tbl, row8);
    table_add(tbl, row9);
    puts("=== Table Add ===");
    print_table(tbl);
    puts("\n");


    table_remove(tbl, "fname", "Noah");
    puts("=== Table Remove ===");
    print_table(tbl);
    puts("\n");


    puts("=== Table Print Index (\"lname\") ===");
    print_table_index(tbl, "lname");
    puts("\n");


    table_add_index(tbl, "lname");
    puts("=== Table Add Index ===");
    print_table(tbl);
    puts("\n");


    puts("=== Table Print Index (\"lname\") ===");
    print_table_index(tbl, "lname");
    puts("\n");


    table_remove_index(tbl, "lname");
    puts("=== Table Remove Index ===");
    print_table(tbl);
    puts("\n");


    puts("=== Table Print Index (\"lname\") ===");
    print_table_index(tbl, "lname");
    puts("\n");


    table_add(tbl, row10);
    table_add(tbl, row11);
    puts("=== Table Add (Malformed) ===");
    print_table(tbl);
    puts("\n");


    table_remove(tbl, "lname", "Groth");
    puts("=== Table Remove ===");
    print_table(tbl);
    puts("\n");

    database_write(db);

    return 0;
}

