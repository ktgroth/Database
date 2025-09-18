
#ifndef GDB_DATABASE_
#define GDB_DATABASE_

#include "types.h"
#include "dataframe.h"
#include "table.h"


typedef struct
{
    char         *name;

    size_t        capacity;
    size_t        ntables;
    table_t     **tables;
} database_t;


database_t *init_database(const char *name);
void free_database(database_t *db);
void print_database(const database_t *db);


int database_write(database_t *db);
database_t *database_read_path(const char *filepath);
database_t *database_read(FILE *fp);


int database_insert_table(database_t *db, const char *name, size_t ncols, const char **colnames, const type_e *coltypes,
        int indexed, const char *pkname, const type_e pktype);
int database_add_table(database_t *db, table_t *tbl);
int database_remove_table(database_t *db, const char *name);


int database_add_index(database_t *db, const char *tname, const char *colname);
int database_add_table_index(database_t *db, table_t *tbl, const char *colname);
int database_remove_index(database_t *db, const char *tname, const char *colname);
int database_remove_table_index(database_t *db, table_t *tbl, const char *colname);


int database_insert(database_t *db, const char *tname, void **values);
int database_table_insert(database_t *db, table_t *tbl, void **values);
int database_add(database_t *db, const char *tname, datablock_t *row);
int database_table_add(database_t *db, table_t *tbl, datablock_t *row);
const dataframe_t *database_lookup(database_t *db, const char *tname, const char *colname, void *value);
const dataframe_t *database_table_lookup(database_t *db, const table_t *tbl, const char *colname, void *value);
int database_update(database_t *db, const char *tname, const char *keyname, const void *keyval, const char *colname, void *value);
int database_table_update(database_t *db, table_t *tbl, const char *keyname, const void *keyval, const char *colname, void *value);

#endif

