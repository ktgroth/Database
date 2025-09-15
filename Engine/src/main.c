
#include <stdio.h>

#include "include/table.h"


#define UNUSED(x) (void)(x);


int main(int argc, char *argv[])
{
    UNUSED(argc)
    UNUSED(argv);

    const char *colnames[] = { "fname", "lname", "age", "salary" };
    const type_e coltypes[] = { COL_STRING, COL_STRING, COL_INT32, COL_FLOAT64 };
    table_t *tbl = init_table(4, colnames, coltypes, 1, "id", COL_INT64);

    // dataframe_t *frame = init_frame(4, colnames, coltypes);

    void *block1[] = { "Kasen", "Groth", (int []){ 22 }, (double []){ 65000 } };
    void *block2[] = { "Nuno", "Alves", (int []){ 22 }, (double []){ 60000 } };
    void *block3[] = { "Christian", "Groth", (int []){ 23 }, (double []){ 75000 } };
    void *block4[] = { "Joseph", "Speidel", (int []){ 22 }, (double []){ 100000 } };
    void *block5[] = { "Camryn", "Groth", (int []){ 26 }, (double []){ 68000 } };
    void *block6[] = { "Katie", "Smith", (int []){ 28 }, (double []){ 48000 } };
    void *block7[] = { "Noah", "Smith", (int []){ 27 }, (double []){ 88000 } };
    void *block8[] = { "Kyle", "Hammermueller", (int []){ 23 }, (double []){ 100000 } };
    void *block9[] = { "Liam", "Boyle", (int []){ 22 }, (double []){ 98000 } };

    datablock_t *row1 = init_block(tbl->ncols, colnames, coltypes, block1);
    datablock_t *row2 = init_block(tbl->ncols, colnames, coltypes, block2);
    datablock_t *row3 = init_block(tbl->ncols, colnames, coltypes, block3);
    datablock_t *row4 = init_block(tbl->ncols, colnames, coltypes, block4);
    datablock_t *row5 = init_block(tbl->ncols, colnames, coltypes, block5);
    datablock_t *row6 = init_block(tbl->ncols, colnames, coltypes, block6);
    datablock_t *row7 = init_block(tbl->ncols, colnames, coltypes, block7);
    datablock_t *row8 = init_block(tbl->ncols, colnames, coltypes, block8);
    datablock_t *row9 = init_block(tbl->ncols, colnames, coltypes, block9);

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

    return 0;
}

