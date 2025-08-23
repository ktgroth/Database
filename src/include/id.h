
#ifndef GDB_ID_
#define GDB_ID_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>


#define LT(x, y)    (cmp(x, y) < 0) 
#define LTE(x, y)   (cmp(x, y) <= 0)
#define EQ(x, y)    (cmp(x, y) == 0)
#define GTE(x, y)   (cmp(x, y) >= 0)
#define GT(x, y)    (cmp(x, y) > 0)

enum
{
    ID_INT,
    ID_CHAR,
};

typedef struct
{
    int type;
    union
    {
        size_t iid;
        char  *cid;
    };
} db_id_t;


#define id(x) \
    _Generic((x), \
        int:        idi, \
        size_t:     idi, \
        char *:     ids  \
    )(x)

static inline db_id_t ids(char *id)
{
    return (db_id_t){
        .type = ID_CHAR,
        .cid = id
    };
}

static inline db_id_t idi(size_t id)
{
    return (db_id_t){
        .type = ID_INT,
        .iid = id
    };
}

static inline db_id_t idd(void * id, int is_int)
{
    if (is_int)
        return idi(*(size_t *)id);

    return ids((char *)id);
}


#define cmp_size_t(x, y) ((x) > (y) ? 1 : (x) < (y) ? -1 : 0)
#define cmp_str_t(x, y)  strcmp((x), (y))

inline int cmp(db_id_t x, db_id_t y)
{
    assert(x.type == y.type);

    switch (x.type)
    {
        case ID_INT:    return cmp_size_t(x.iid, y.iid);
        case ID_CHAR:   return cmp_str_t(x.cid, y.cid);
    }

    assert(0);
}

void print_id(db_id_t id, char *buf, size_t bufsize);

#endif

