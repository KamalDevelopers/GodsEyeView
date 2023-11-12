#include "errno.h"
#include "math.h"

const char* error_what(int e)
{
    e = abs(e);

    const char* error_table[] = {
        DESC_E_UNKNOWN,
        DESC_E_PERMISSION,
        DESC_E_VSFENTRY,
        DESC_E_UNKNOWN,
        DESC_E_UNKNOWN,
        DESC_E_UNKNOWN,
        DESC_E_UNKNOWN,
        DESC_E_UNKNOWN,
        DESC_E_INVALIDEXEC,
        DESC_E_INVALIDFD,
        DESC_E_NOCHILD,
    };

    if (e >= (sizeof(error_table) / sizeof(char*)))
        return DESC_E_UNKNOWN;
    return error_table[e];
}
