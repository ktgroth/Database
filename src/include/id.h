
#ifndef GDB_ID_
#define GDB_ID_

#include <assert.h>

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

#define cmp_size_t(x, y) ((x) > (y) ? 1 : (x) < (y) ? -1 : 0)
#define cmp_str_t(x, y)  strcmp((x), (y))

static inline int cmp(db_id_t x, db_id_t y)
{
    assert(x.type == y.type);

    switch (x.type)
    {
        case ID_INT:    return cmp_size_t(x.iid, x.iid);
        case ID_CHAR:   return cmp_str_t(x.cid, y.cid);
    }

    assert(0);
}

#endif

