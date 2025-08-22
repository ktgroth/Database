
#include "include/id.h"

db_id_t ids(char *id)
{
    return (db_id_t){
        .type=ID_CHAR,
        .cid=id
    };
}

db_id_t idi(int id)
{
    return (db_id_t){
        .type=ID_INT,
        .iid=id
    };
}

