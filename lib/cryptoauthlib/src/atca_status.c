#include "atca_status.h"

const char *atca_status_name(ATCA_STATUS status)
{
    #define DEF_STATUS_CODE(_name, _value) case _name: return #_name;
    switch (status) {
    STATUS_CODES
    default:
        return NULL;
    }
    #undef DEF_STATUS_CODE
}