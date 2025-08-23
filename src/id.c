
#include "include/id.h"

void print_id(db_id_t id, char *buf, size_t bufsize)
{
    switch (id.type)
    {
        case ID_INT:
            snprintf(buf, bufsize, "%ld", id.iid);
            break;

        case ID_CHAR:
            snprintf(buf, bufsize, "%s", id.cid);
            break;

        default:
            snprintf(buf, bufsize, "<invalid>");
    }
}

