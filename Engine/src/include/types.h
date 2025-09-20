
#ifndef GDB_TYPES_
#define GDB_TYPES_

#include <stdio.h>

#include <stdint.h>


typedef enum
{
    COL_NULL        = 0,
    COL_BOOL        = 1,
    COL_INT8        = 2,
    COL_INT16       = 3,
    COL_INT32       = 4,
    COL_INT64       = 5,
    COL_FLOAT32     = 6,
    COL_FLOAT64     = 7,
    COL_STRING      = 8,
    COL_DATETIME    = 9,
    COL_BLOB        = 10,

    KEY_PK          = 1 << 30,
    KEY_FK          = 1 << 31,
} type_e;

typedef struct
{
    size_t   size;
    char    *data;
} blob_t;

typedef struct
{
    type_e  type;
    union
    {
        int8_t      *b;
        int8_t      *i8;
        int16_t     *i16;
        int32_t     *i32;
        int64_t     *i64;
        float       *f32;
        double      *f64;
        char        *str;
        char        *datetime;
        blob_t      *blob;
        void        *value;
    };
} datafield_t;


#define      LT(type, a, b)     value_cmp((type), (a), (b)) <  0
#define      LTE(type, a, b)    value_cmp((type), (a), (b)) <= 0
#define      EQ(type, a, b)     value_cmp((type), (a), (b)) == 0
#define      NEQ(type, a, b)    value_cmp((type), (a), (b)) != 0
#define      GTE(type, a, b)    value_cmp((type), (a), (b)) >= 0
#define      GT(type, a, b)     value_cmp((type), (a), (b)) >  0

void         print_value(type_e type, const void *value);
int          value_cmp(type_e type, const void *a, const void *b);


datafield_t *init_field(type_e type, const void *value);
void         free_field(datafield_t *field);


int          field_cmp(const datafield_t *f1, const datafield_t *f2);
int          field_cmp_value(const datafield_t *field, const void *value);
int          field_update(datafield_t *field, const void *value);


blob_t      *construct_blob_path(const char *path);
blob_t      *construct_blob_file(FILE *blob);
void         free_blob(blob_t *blob);

#endif

