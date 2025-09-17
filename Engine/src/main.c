
#include <stdio.h>

#include "include/table.h"


#define UNUSED(x) (void)(x);


int main(int argc, char *argv[])
{
    UNUSED(argc)
    UNUSED(argv);

    const char *colnames[] = { "fname", "lname", "age", "salary" };
    const type_e coltypes[] = { COL_STRING, COL_STRING, COL_INT32, COL_FLOAT64 };
    table_t *tbl = init_table("people", 4, colnames, coltypes, 1, "id", COL_INT64);

    void *block1[] = { "Kasen", "Groth", (int []){ 22 }, (double []){ 65000 } };
    void *block2[] = { "Nuno", "Alves", (int []){ 22 }, (double []){ 60000 } };
    void *block3[] = { "Christian", "Groth", (int []){ 23 }, (double []){ 75000 } };
    void *block4[] = { "Joseph", "Speidel", (int []){ 22 }, (double []){ 100000 } };
    void *block5[] = { "Camryn", "Groth", (int []){ 26 }, (double []){ 68000 } };
    void *block6[] = { "Katie", "Smith", (int []){ 28 }, (double []){ 48000 } };
    void *block7[] = { "Noah", "Smith", (int []){ 27 }, (double []){ 88000 } };
    void *block8[] = { "Kyle", "Hammermueller", (int []){ 23 }, (double []){ 100000 } };
    void *block9[] = { "Liam", "Boyle", (int []){ 22 }, (double []){ 98000 } };

    const char *colnames2[] = { "fname", "lname", "nkids", "salary", "age" };
    const type_e coltypes2[] = { COL_STRING, COL_STRING, COL_INT8, COL_FLOAT64, COL_INT32 };
    void *block10[] = { "Heidi", "Groth", (int8_t []){ 4 }, (double []){ 50000 }, (int []){ 62 } };
    void *block11[] = { "Richard", "Groth", (int8_t []){ 4 }, (double []){ 250000 }, (int []){ 57 } };

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


    dataframe_t *groths = table_lookup(tbl, "lname", "Groth");
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


    return 0;
}

