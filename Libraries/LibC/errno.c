#include "errno.h"

const char* error_what(int e)
{
    if (e >= 0)
        return "none";

    const char* error_table[] = {
        "none",
        DESC_E_PERMISSION,
        DESC_E_VSFENTRY,
        DESC_E_UNKNOWN,
        DESC_E_UNKNOWN,
        DESC_E_UNKNOWN,
        DESC_E_UNKNOWN,
        DESC_E_UNKNOWN,
        DESC_E_INVALIDEXEC,
        DESC_E_INVALIDFD,
    };

    if (-e >= (sizeof(error_table) / sizeof(char*)))
        return DESC_E_UNKNOWN;
    return error_table[-e];
}
